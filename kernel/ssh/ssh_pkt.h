
#ifndef	__SSH_PKT_H_
#define __SSH_PKT_H_


//packet func
Packet* ssh_new_packet(void);
Packet* ssh2_pkt_init(int pkt_type);
Packet* ssh_read_packet(ssh_session* ssh,unsigned char* data,int* len);
void    ssh_free_packet(Packet *pkt);
void    ssh2_pkt_send(ssh_session* ssh, Packet *pkt);

void    ssh2_pkt_send_noqueue(ssh_session* ssh,  Packet *pkt);
void    ssh_pkt_addbyte( Packet *pkt, unsigned char byte);
void    ssh_pkt_adduint32( Packet *pkt, unsigned long value);
void    ssh_pkt_addstring( Packet *pkt, const char *data);
void    ssh_pkt_getstring(Packet *pkt, char **p, int *length);
void    ssh2_pkt_addbool( Packet *pkt, unsigned char value);
void    ssh_pkt_addstring_str( Packet *pkt, const char *data);
void    ssh_pkt_addstring_data( Packet *pkt, const char *data, int len);
void    ssh_pkt_addstring_start( Packet *pkt);
void    ssh2_pkt_queuesend(ssh_session* ssh);
Bignum  ssh2_pkt_getmp(Packet *pkt);
void    ssh2_pkt_addmp(Packet *pkt, Bignum b);

void    ssh2_pkt_send_with_padding(ssh_session*  ssh,  Packet *pkt,int padsize);

unsigned long ssh_pkt_getuint32( Packet *pkt);

unsigned char *ssh2_mpint_fmt(Bignum b, int *len);

/* For legacy code (SSH-1 and -2 packet construction used to be separate) */
#define ssh2_pkt_ensure(pkt, length) ssh_pkt_ensure(pkt, length)
#define ssh2_pkt_adddata(pkt, data, len) ssh_pkt_adddata(pkt, data, len)
#define ssh2_pkt_addbyte(pkt, byte) ssh_pkt_addbyte(pkt, byte)
#define ssh2_pkt_adduint32(pkt, value) ssh_pkt_adduint32(pkt, value)
#define ssh2_pkt_addstring_start(pkt) ssh_pkt_addstring_start(pkt)
#define ssh2_pkt_addstring_str(pkt, data) ssh_pkt_addstring_str(pkt, data)
#define ssh2_pkt_addstring_data(pkt, data, len) ssh_pkt_addstring_data(pkt, data, len)
#define ssh2_pkt_addstring(pkt, data) ssh_pkt_addstring(pkt, data)



#endif //