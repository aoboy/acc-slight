

/* Must disable interrupt driven output when testing rtimer!
   Otherwise serial output from rtimer will block forever
   (no uart interrupt can run to empty the uart buffer).
  */
#undef UART1_CONF_TX_WITH_INTERRUPT
#define UART1_CONF_TX_WITH_INTERRUPT 0

/* Use nullmac during rtimer tests to avoid interference */
//#undef NETSTACK_CONF_MAC
//#define NETSTACK_CONF_MAC nullmac_driver
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC accsl_driver

/* Disable dco synch for now because it might affect the rtimers */
//#undef DCOSYNCH_CONF_ENABLED
//#define DCOSYNCH_CONF_ENABLED 0
//#include "m_rtimer.h"
