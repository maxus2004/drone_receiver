void crsf_sendChannels(void* channels);
void crsf_begin();

struct __attribute__ ((packed)) crsf_channels_s {
    unsigned roll : 11;
    unsigned pitch : 11;
    unsigned thrust : 11;
    unsigned yaw : 11;
    unsigned aux1 : 11;
    unsigned aux2 : 11;
    unsigned aux3 : 11;
    unsigned aux4 : 11;
    unsigned aux5 : 11;
    unsigned aux6 : 11;
    unsigned aux7 : 11;
    unsigned aux8 : 11;
    unsigned aux9 : 11;
    unsigned aux10 : 11;
    unsigned aux11 : 11;
    unsigned aux12 : 11;
};