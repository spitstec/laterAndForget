

 /* This free software comes without any warranty, to
 the extent permitted by applicable law. */
 
 
// times must be expressed in an unsigned type. unsigned long, unsigned int or unsigned char
#ifndef TIMTYP
typedef  unsigned long TIMTYP ;
#define ETERNAL 0xffffffff
#endif

typedef void (*t_event)()  ;

// optional payload system
#ifdef withpayload 

#ifndef payloadtype
#define payloadtype int
#endif
payloadtype payload ;
#endif

// struct that is used in the timing queue
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

// normally there are max 20 positions in the timing queue can be changed by defining "pevheapsize"
#ifndef pevheapsize
#define pevheapsize 20
#endif

// simulated heap with timing nodes
teventnode pevheap[pevheapsize] ;


int pevcnt ;

// execute event when not NULL
void executeEvent(t_event ev) {
  if (ev != NULL)
    (*ev)() ; }

// free timing node back to heap
void pevfree(void * p)  {
  ((teventnode *)p)->link = pevtop ;
  pevtop = (teventnode *) p ;
  pevcnt-- ;
}


// allocate timing node 
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



// forget function: searches timing queue for event. if found it removes one item from the queue
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
// this forget function looks for payload/event combination
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

// this system uses one basic timer fuction either milliseconds or microseconds
#ifdef microsLater
#define nowtime() (TIMTYP) (micros())
#else
#define nowtime() (TIMTYP) (millis())
#endif

// "reftime"  is used to find events that should be started in the past
TIMTYP reftime ;

//later function put one new time int the timing queue. The que is sorted based on times
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
      if ((TIMTYP)(hev->firetime-reftime) > delaytim) {
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

//later function without removing existing events from queue
void alsoLater(TIMTYP delaytim,t_event ev) {
  dontkillevent = true ;
  later(delaytim,ev) ;
  dontkillevent = false ; }

// returns time until a particular event occurs. Large number if event not in queue
TIMTYP timeUntil(t_event ev) {
  TIMTYP rslt = ETERNAL;
  TIMTYP n ;
  pevnode hev = eventlist ;
  while (hev != NULL) {
    if (hev->eve == ev) {
      n = hev->firetime-nowtime() ;
      if (n < rslt)
        rslt = n ; }
    hev = (pevnode) hev->link ; } 
  return rslt ; }

// last event that is started. Can be used to loop back to same event
t_event current_event ;

// check the queue for event of which it is time to execute. events that should have been executed in the past are
// executed first
bool peek_event(void) {
  pevnode hev ;
  current_event = (t_event) NULL ;
  if (eventlist == NULL) {
    reftime = nowtime() ;
    return false ; }
  else {
    TIMTYP clock = nowtime() ;
    if ((TIMTYP)(clock-reftime) < (TIMTYP)(eventlist->firetime-reftime))
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


// macro to check for input change. a static bool that starts with "was" is created for each input that is tested
// these macros are created for situations where onChange() ,onRise() or onFall() are used
#define checkinp(x) static bool was##x ; if (was##x != digitalRead(x)) { was##x = !was##x ; if (was##x) onRise_##x(); else onFall_##x();}
#define checkinpl(x) static bool was##x ; if (was##x != digitalRead(x)) { was##x = !was##x ; if (!was##x) onFall_##x();}
#define checkinph(x) static bool was##x ; if (was##x != digitalRead(x)) { was##x = !was##x ; if (was##x) onRise_##x();}
#define checkChange(x) static bool was##x ; if (was##x != digitalRead(x)) { was##x = !was##x ; onChange_##x(was##x);}

//onRise, onFall and OnChange macros are converted to valid c functions
#define onRise(xy) void onRise_##xy(void)
#define onFall(xy) void onFall_##xy(void)
#define onChange(xy) void onChange_##xy(bool hi)
//addSetup keyword is converted to if(0) so that next statement is not executed but still syntax checked by c compiler
#define addSetup if (0)
// glob_ is used to create global variables by adding one word (ie glob_int) before an identifier
// these 2 keywords are neutralized for the ppc compiler
#define glob_bool
#define glob_int
