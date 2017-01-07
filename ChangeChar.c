#include <iconv.h>

int code_convert(char *from_charset, char *to_charset, char *inbuf, int inlen, char *outbuf, int outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;
    cd = iconv_open(to_charset, from_charset);
    if( cd == 0 ) return -1;
    memset( outbuf, 0 , outlen);
    if( iconv(cd, pin, &inlen, pout, &outlen) == -1 ) return -1;
    iconv_close(cd);
    return 0;
}

int unicode_gb2312(char *inbuf, int inlen, char *outbuf, int outlen)
{
    return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
}
int gb2312_unicode(char *inbuf, int inlen, char *outbuf, int outlen)
{
    return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
}
