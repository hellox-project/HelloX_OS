# AUTOMAKE_OPTIONS = subdir-objects
include $(top_srcdir)/kernel/kernel.mk

noinst_LIBRARIES = libnetwork.a

libnetwork_a_SOURCES = ./netif/ethernetif.c \
./netif/slipif.c                             \
./netif/etharp.c                             \
./core/dns.c                                 \
./core/pbuf.c                                \
./core/def.c                                 \
./core/stats.c                               \
./core/sys.c                                 \
./core/memp.c                                \
./core/icmp.c                                \
./core/ip_frag.c                             \
./core/ip.c                                  \
./core/inet.c                                \
./core/udp.c                                 \
./core/tcp_out.c                             \
./core/inet_chksum.c                         \
./core/init.c                                \
./core/ip_addr.c                             \
./core/timers.c                              \
./core/netif.c                               \
./core/mem.c                                 \
./core/raw.c                                 \
./core/dhcp.c                                \
./core/tcp.c                                 \
./core/autoip.c                              \
./core/tcp_in.c                              \
./core/igmp.c                                \
./arch/sys_arch.c                            \
./api/tcpip.c                                \
./api/netbuf.c                               \
./api/err.c                                  \
./api/netdb.c                                \
./api/api_msg.c                              \
./api/api_lib.c                              \
./api/sockets.c                              \
./api/netifapi.c                             \
./ethernet/ethentry.c                        \
./ethernet/ethif.c
