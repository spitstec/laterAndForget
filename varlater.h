
 /* This free software comes without any warranty, to
 the extent permitted by applicable law. */

/* this version of later() has separate functions for microseconds,milliseconds and seconds */

typedef void (*t_event)()  ;

#ifdef withpayload

#ifndef payloadtype
#define payloadtype int
#endif
payloadtype payload ;
#endif

#ifndef TIMTYP
#define TIMTYP unsigned long
#define ETERNAL 0xffffffff
#endif

typedef struct teventnode {
   t_event  eve ;
   #ifdef withpayload
   payloadtype load ;
   #endif
   TIMTYP firetime;
   teventnode  *  link ; } teventnode;
#define pevnode teventnode *

pevnode eventlist[2] = {NULL,NULL} ;
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



pevnode secEvents = NULL ;
TIMTYP secCount ;  
bool secondsForget(t_event ev) ;


bool forget(t_event ev) {
  bool rslt = false ;
  if (secEvents && secondsForget(ev))
    return true ;
  for (int idx=0;idx<=(1);idx++) {
    pevnode prev = NULL ;
    pevnode hev = eventlist[idx] ;
    while (hev != NULL) {
      if (hev->eve == ev) {
        rslt = true ;
        if (prev) {
          prev->link = hev->link ;
          pevfree(hev) ;
          hev = NULL ; }
        else {
          prev = eventlist[idx] ;
          eventlist[idx] = eventlist[idx]->link ;
          pevfree(prev) ;
          hev = NULL ; } 
        idx = 99 ;  }
      else {
         prev = hev ;
         hev = (pevnode) hev->link ; } } }
  return rslt ;
}

bool dontkillevent = false ;




TIMTYP reftime[2] ;

void varLater(TIMTYP delaytim,t_event ev,int microItv) {
  pevnode nev ;
  pevnode hev ;
  pevnode prev ;
  //printf("(%d %02X) ",delay,ev) ;
  if (!dontkillevent)
    forget(ev) ;
  nev = (pevnode)pevmalloc() ;
  TIMTYP now = microItv?micros():millis() ;
  if (eventlist[microItv] == NULL)
    reftime[microItv] = now ;
  if (nev != NULL) {
    nev->link = NULL ;
    #ifdef withpayload
    nev->load = payload ;
    #endif
    nev->eve = (t_event) ev ;
    nev->firetime = now+delaytim ;
    delaytim = nev->firetime-reftime[microItv] ;
    hev = (pevnode)eventlist[microItv] ;
    prev = NULL ;
    if (hev == NULL) {
      eventlist[microItv] = nev ; }
    while (hev != NULL) {
      if (hev->firetime-reftime[microItv] > (TIMTYP)delaytim) {
        if (prev == NULL) {
          nev->link = eventlist[microItv] ;
          eventlist[microItv] = nev ; }
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


void later(TIMTYP delaytim,t_event ev) {
  varLater(delaytim,ev,0) ; }

void microsLater(TIMTYP delaytim,t_event ev) {
  varLater(delaytim,ev,1) ; }

void alsoLater(TIMTYP delaytim,t_event ev) {
  dontkillevent = true ;
  varLater(delaytim,ev,0) ; 
  dontkillevent = false ; }

void alsoMicrosLater(TIMTYP delaytim,t_event ev) {
  dontkillevent = true ;
  varLater(delaytim,ev,1) ; 
  dontkillevent = false ; }

TIMTYP timeUntil(t_event ev,int microItv) {
  TIMTYP rslt = ETERNAL ;
  int n ;
  TIMTYP now = microItv?micros():millis() ;
  pevnode hev = eventlist[microItv] ;
  while (hev != NULL) {
    if (hev->eve == ev) {
      n = hev->firetime-now ;
      if (n < rslt)
        rslt = n ; }
    hev = (pevnode) hev->link ; } 
  return rslt ; }


t_event current_event ;

bool peek_event(void) {
  pevnode hev ;
  current_event = (t_event) NULL ;
  for (int idx=1;idx>=0;idx--) {
    TIMTYP clock = idx?micros():millis() ;
    if (eventlist[idx] == NULL) {
      reftime[idx] = clock ;}
    else {
      if ((TIMTYP)(clock-reftime[idx]) < (TIMTYP)(eventlist[idx]->firetime-reftime[idx]))
        reftime[idx] = clock ;
      else {
        reftime[idx] = eventlist[idx]->firetime ;
        current_event = eventlist[idx]->eve ;
        #ifdef withpayload
        payload = eventlist[idx]->load ;
        #endif
        hev = eventlist[idx] ;
        eventlist[idx]= (pevnode) eventlist[idx]->link ;
        pevfree(hev) ;
        idx = -99 ;
        } } }
  if (current_event)
     (*current_event)() ;   
  return (current_event != NULL) ; }
  

void oneSecond(void) {
  secCount++ ;
  pevnode nev = secEvents ;
  while (nev) {
    pevnode prev = NULL ;
    current_event = (t_event) NULL ;
    while (nev && !current_event) {
      if (nev->firetime == secCount) {
        current_event = nev->eve ;
        #ifdef withpayload
        payload = nev->load ;
        #endif
        }
      else {
        prev = nev ;
        nev = nev->link ; } }
    if (current_event) {
      if (prev)
        prev->link = nev->link ;
      else
        secEvents = nev->link ;
      pevfree(nev) ;  
      (*current_event)() ; 
      nev = secEvents ; } }
  if (secEvents)
    later(1000,oneSecond) ;
 }
      
void secondsLater(TIMTYP delaytim,t_event ev) {
  if (delaytim == 0) 
    delaytim = 1 ;
  if (secEvents == NULL)
    later(1000,oneSecond) ;
  pevnode nev ;
  nev = pevmalloc() ;
  nev->firetime = secCount+delaytim ;
  nev->eve = ev ;
  nev->link = NULL ;
  #ifdef withpayload
  nev->load = payload ;
  #endif 
  nev->link = secEvents ;
  secEvents = nev ;
}


bool secondsForget(t_event ev) {
  bool rslt = false ;
  pevnode nev = secEvents ;
  pevnode prev = NULL ;
  while (nev) {
    if (nev->eve == ev) {
      rslt = true ;
      if (prev)
        prev->link = nev->link ;
      else
        secEvents = nev->link ;
      pevfree(nev) ;  
      nev = NULL ; }
    else {
      prev = nev ;
      nev = nev->link ; } }
  return rslt ;
}
  



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
