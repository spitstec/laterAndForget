

#ifndef TIMTYP
#define TIMTYP unsigned long
#define ETERNAL 0xffffffff
#endif

typedef void (*t_event)()  ;

#ifdef withpayload 

#ifndef payloadtype
#define payloadtype int
#endif
payloadtype payload ;
#endif
typedef struct teventnode {
   t_event  eve ;
   #ifdef withpayload
   payloadtype load ;
   #endif
   TIMTYP firetime;
   teventnode  *  link ; } teventnode;
#define pevnode teventnode *

pevnode eventlist = NULL ;
pevnode pevtop = NULL ;
bool pevinit = false ;
#ifndef pevheapsize
#define pevheapsize 20
#endif
teventnode pevheap[pevheapsize] ;


int pevcnt ;

void executeEvent(t_event ev) {
  if (ev != NULL)
    (*ev)() ; }


void pevfree(void * p)  {
  ((teventnode *)p)->link = pevtop ;
  pevtop = (teventnode *) p ;
  pevcnt-- ;
}


teventnode * pevmalloc(void) {
  int i ;
  if (!pevinit) {
    for(i=(0);i<=(pevheapsize-2);i++) {
      pevheap[i].link = &pevheap[i+1] ;
      pevheap[i].eve = NULL ; }
    pevheap[pevheapsize-1].link = NULL ;
    pevtop = (teventnode *) &pevheap[0] ;
    pevinit = true ; }
  teventnode * tmp = pevtop ;
  pevcnt++ ;
  if (pevtop != NULL)
    pevtop = (teventnode *) pevtop->link ;
  return tmp ;
}




bool forget(t_event ev) {
  pevnode prev = NULL ;
  pevnode hev = eventlist ;
  bool rslt = false ;
  while (hev != NULL) {
    if (hev->eve == ev) {
      rslt = true ;
      if (prev) {
        prev->link = hev->link ;
        pevfree(hev) ;
        hev = NULL ; }
      else {
        prev = eventlist ;
        eventlist = eventlist->link ;
        pevfree(prev) ;
        hev = NULL ; } }
     else {
       prev = hev ;
       hev = (pevnode) hev->link ; } }
  return rslt ;
}

#ifdef withpayload
bool plForget(t_event ev,payloadtype pl) {
  pevnode prev = NULL ;
  pevnode hev = eventlist ;
  bool rslt = false ;
  while (hev != NULL) {
    if (hev->eve == ev && hev->load == pl) {
      rslt = true ;
      if (prev) {
        prev->link = hev->link ;
        pevfree(hev) ;
        hev = prev->link ; }
      else {
        prev = eventlist ;
        eventlist = eventlist->link ;
        pevfree(prev) ;
        prev = NULL ;
        hev = eventlist ; } }
     else {
       prev = hev ;
       hev = (pevnode) hev->link ; } }
  return rslt ;
}
#endif

bool dontkillevent = false ;


#ifdef microsLater
#define nowtime() (TIMTYP) (micros())
#else
#define nowtime() (TIMTYP) (millis())
#endif


TIMTYP reftime ;

void later(TIMTYP delaytim,t_event ev) {
  pevnode nev ;
  pevnode hev ;
  pevnode prev ;
  //printf("(%d %02X) ",delay,ev) ;
  if (!dontkillevent)
    forget(ev) ;
  nev = (pevnode)pevmalloc() ;
  if (eventlist == NULL)
    reftime = nowtime() ;
  if (nev != NULL) {
    nev->link = NULL ;
    #ifdef withpayload
    nev->load = payload ;
    #endif
    nev->eve = (t_event) ev ;
    nev->firetime = nowtime()+delaytim ;
    delaytim = nev->firetime-reftime ;
    hev = (pevnode)eventlist ;
    prev = NULL ;
    if (hev == NULL) {
      eventlist = nev ; }
    while (hev != NULL) {
      if (hev->firetime-reftime > delaytim) {
        if (prev == NULL) {
          nev->link = eventlist ;
          eventlist = nev ; }
        else {
          nev->link = hev ;
          prev->link = nev ; }
        hev = NULL ; }
      else {
        prev = hev ;
        hev = (pevnode) hev->link ;
        if (hev == NULL)
          prev->link = nev ;} } }
}

void alsoLater(TIMTYP delaytim,t_event ev) {
  dontkillevent = true ;
  later(delaytim,ev) ;
  dontkillevent = false ; }

TIMTYP timeUntil(t_event ev) {
  TIMTYP rslt = ETERNAL;
  int n ;
  pevnode hev = eventlist ;
  while (hev != NULL) {
    if (hev->eve == ev) {
      n = hev->firetime-nowtime() ;
      if (n < rslt)
        rslt = n ; }
    hev = (pevnode) hev->link ; } 
  return rslt ; }


t_event current_event ;

bool peek_event(void) {
  pevnode hev ;
  current_event = (t_event) NULL ;
  if (eventlist == NULL) {
    reftime = nowtime() ;
    return false ; }
  else {
    TIMTYP clock = nowtime() ;
    if (clock-reftime < eventlist->firetime-reftime)
      reftime = clock ;
    else {
      reftime = eventlist->firetime ;
      current_event = eventlist->eve ;
      #ifdef withpayload
      payload = eventlist->load ;
      #endif
      hev = eventlist ;
      eventlist = (pevnode) eventlist->link ;
      pevfree(hev) ;} }
  if (current_event)
     (*current_event)() ;   
  return (current_event != NULL) ; }

#define checkinp(x) static bool was##x ; if (was##x != digitalRead(x)) { was##x = !was##x ; if (was##x) onRise_##x(); else onFall_##x();}
#define checkinpl(x) static bool was##x ; if (was##x != digitalRead(x)) { was##x = !was##x ; if (!was##x) onFall_##x();}
#define checkinph(x) static bool was##x ; if (was##x != digitalRead(x)) { was##x = !was##x ; if (was##x) onRise_##x();}
#define checkChange(x) static bool was##x ; if (was##x != digitalRead(x)) { was##x = !was##x ; onChange_##x(was##x);}
#define onRise(xy) void onRise_##xy(void)
#define onFall(xy) void onFall_##xy(void)
#define onChange(xy) void onChange_##xy(bool hi)
#define addSetup if (0)
#define glob_bool
#define glob_int
