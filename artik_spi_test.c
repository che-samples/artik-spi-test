#include <stdio.h>
#include <artik_module.h>
#include <artik_platform.h>
#define COMMAND_REG         0x01
#define COMMIEN_REG         0x02
#define CMD_SOFTRESET       0x0F
#define READ_REG(reg)       ((reg<<1) | 0x80)
#define WRITE_REG(reg)      (reg<<1)
static artik_spi_config config = {
        1,
        0,
        SPI_MODE0,
        8,
        500000,
};
artik_error spi_test(int platid)
{
    artik_spi_module* spi = (artik_spi_module*)artik_get_api_module("spi");
    artik_spi_handle pn512;
    artik_error ret;
    unsigned char tx_buf[2] = {0, };
    unsigned char rx_buf[2] = {0, };
    int i;
    fprintf(stdout, "TEST: %s starting\n", __func__);
    ret = spi->request(&pn512, &config);
    if (ret != S_OK) {
        fprintf(stderr, "Failed to request SPI %d\n",ret);
        goto exit;
    }
    fprintf(stdout, "Send Soft Reset command...\n");
    
    tx_buf[0] = WRITE_REG(COMMAND_REG);
    tx_buf[1] = CMD_SOFTRESET;
 
    ret = spi->read_write(pn512, tx_buf, rx_buf, sizeof(tx_buf));
    if (ret != S_OK) {
            fprintf(stderr, "FAILED\nFailed to write spidev%d.%d register 0x%04x (%d)\n", config.bus, config.cs, COMMAND_REG, ret);
            goto exit;
    }
    fprintf(stdout, "Read commien register...\n");
    tx_buf[0] = READ_REG(COMMIEN_REG);
    tx_buf[1] = 0;
    
    ret = spi->read_write(pn512, tx_buf, rx_buf, sizeof(rx_buf));
    if (ret != S_OK) {
            fprintf(stderr, "FAILED\nFailed to read spidev%d.%d register 0x%04x (%d)\n", config.bus, config.cs, COMMIEN_REG, ret);
            goto exit;
    }
    if(rx_buf[1] != 0x80) {
        fprintf(stderr, "%s: Wrong value read, expected 0x80, got 0x%02x\n", __func__, rx_buf[1]);
        ret = E_BAD_ARGS;
    } else {
        fprintf(stdout, "OK - val=0x%02x\n", rx_buf[1]);
    }
    
    fprintf(stdout, "Writing 0x11 to commien register\n");
    
    tx_buf[0] = WRITE_REG(COMMIEN_REG);
    tx_buf[1] = 0x11;
        
    ret = spi->read_write(pn512, tx_buf, rx_buf, sizeof(tx_buf));
    if (ret != S_OK) {
            fprintf(stderr, "FAILED\nFailed to write spidev%d.%d register 0x%04x (%d)\n", config.bus, config.cs, COMMIEN_REG, ret);
            goto exit;
    }
    
    fprintf(stdout, "Reading commien register\n");
    tx_buf[0] = READ_REG(COMMIEN_REG);
    tx_buf[1] = 0;
    
    ret = spi->read_write(pn512, tx_buf, rx_buf, sizeof(rx_buf));
    if (ret != S_OK) {
            fprintf(stderr, "FAILED\nFailed to read spidev%d.%d register 0x%04x (%d)\n", config.bus, config.cs, COMMIEN_REG, ret);
            goto exit;
    }
    
    if(rx_buf[1] != 0x11) {
        fprintf(stderr, "%s: Wrong value read, expected 0x11, got 0x%02x\n", __func__, rx_buf[1]);
        ret = E_BAD_ARGS;
    } else {
        fprintf(stdout, "OK - val=0x%02x\n", rx_buf[1]);
    } 
    ret = spi->release(pn512);
    if (ret != S_OK) {
        fprintf(stderr, "Failed to release spidev%d.%d (%d)\n", config.bus, config.cs, ret);
        goto exit;
    }
exit:
    fprintf(stdout, "TEST: %s %s\n", __func__, (ret == S_OK) ? "succeeded" : "failed");
    return ret;
}
int main()
{
    artik_error ret = S_OK;
    int platid = artik_get_platform();
    if((platid == ARTIK5) || (platid == ARTIK10)) {
        ret = spi_test(platid);
    }
    return (ret == S_OK) ? 0 : -1;
}
