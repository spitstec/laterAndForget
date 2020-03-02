

#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <windows.h>

#define bool unsigned char
#define true 1
#define false 0
#define txtSKIPEXAMP
#define txtPOOLSIZE 16*32768 // available memory for strings
#define txtMAXLEN 4096   // max string length

#include "milkstrict.c"

#define forloop(x,y,z) for (x = (y);x <= (z);x++) 
#define intloop(x,y,z) int x ;for (x = (y);x <= (z);x++) 


FILE * fglob1, *fglob2,*fglob3 ;



bool readstringfrom(tXt fname,tXt * ps ,FILE ** pfi)  {
    if (*pfi == NULL) {
        *pfi = fopen(fname,"r") ; }
    if (*pfi != NULL) {
        char inbuf[txtMAXLEN] ;
        inbuf[0] = 0 ;
        if (fgets(inbuf,txtMAXLEN,*pfi) == NULL)  {
            fclose(*pfi) ;
            *pfi = NULL ; }
        else  {
            int li = strlen(inbuf) ; 
            if (li > 0 && inbuf[li-1]=='\n')  {
                inbuf[li-1]=0 ;  }
            *ps =  txtConcat(inbuf,NULL) ;  }}
    return (*pfi != NULL ) ; }


void writestringto(tXt fname,tXt s , FILE ** pfo)  {
    if (*pfo == NULL) {
        *pfo = fopen(fname,"w+") ; }
    if (*pfo != NULL) {
        if (!s[0]) {
            fprintf(*pfo,"\n") ; }
        else {
            fprintf(*pfo,"%s\n",s) ;  }}}


void finishfile(FILE ** pf)  {
    if (*pf != NULL) {
        fclose(*pf) ; }
    *pf = NULL ;  }

// test file
bool FileExists(tXt nam)  {
    bool rslt = true ;
    FILE * fi = fopen(nam,"r") ;
    if (fi == NULL) {
        rslt = false ; }
    else {
        fclose(fi) ; }
    return rslt ;  }


    

//investigate if char in a range of chars 
bool inset(char c,tXt st)  {
    int i = 0 ;
    bool found = false ;
    while (i < strlen(st) && !found)  {
        found = (c >= st[i] && c <= st[i+1]) ;
        i += 2 ;  }
    return found ; }





// limit an integer
int lim(int x,int mn,int mx) {
  if (x > mx)
    x = mx ;
  if (x < mn)
    x = mn ;
  return x ; }

#define print3int(x,y,z) printf(#x "=%d "#y "=%d "#z "=%d\n",x,y,z) ;
#define EQS(sa,sb) (strcmp(sa,sb)==0)
#define print3string(xa1,xa2,xa3) printf("%s=%s %s=%s %s=%s \n" ,#xa1,xa1 ,#xa2,xa2 ,#xa3,xa3 );
int spacecnt ;
bool commentmode = false ;
bool commenteol = false ;
int commentpos = 999 ;
tXt getctoken(tXt *ps)
{
  int      i;
  tXt res = "" ;
  commenteol = commentmode ;
  if (commentmode) {
    i = txtPos(*ps,"*/") ;
    if (i > 999 )
      i = strlen(*ps)-2 ;
    else {
      commentpos = 9999 ;
      commentmode = false ; }
    commenteol = commentmode ;
    i += 2 ;
    //res = txtSub(*ps,0,i) ;
    res = txtEmpty ;
    *ps = txtDelete(*ps,0,i) ; }
  else {
    spacecnt = 0 ;
    while ((*ps)[0] && ((*ps)[0] == ' ' || (*ps)[0] == '\t')) {
      spacecnt++ ;
      (*ps)++ ; }
    if ((*ps)[0]) {
      if (EQS(txtSub(*ps,0,2),"//")) {
        res = txtConcat(*ps,NULL) ;
        commenteol = true ;
        *ps = txtEmpty ; }
      if (txtPos(*ps,"/*")==0)  {
        i = txtPos(*ps,"*/");
        if (i <999) {
          i += 2 ;
          res = txtSub(*ps,0,i) ;
          (*ps) += i ; }
        else {
          commentmode = true ;
          res = txtConcat(*ps,NULL) ;
          *ps = txtEmpty ; } }
      if (!res[0]) {
        if (inset((*ps)[0] ,"\"\"\'\'")) {
          res = txtSub(*ps,0,1) ;
          (*ps)++ ;
          i = txtPos(*ps,txtC(res[0])) ;
          if (i < 999)
            res = txtConcat(res,txtSub(*ps,0,i+1),NULL) ;
          (*ps)+= i+1 ; }
        else {
          if (inset((*ps)[0] ,"azAZ09$$__")) {
            i = 1 ;
            if (inset((*ps)[0],"09"))
              while (i < strlen(*ps) && inset((*ps)[i], "09.."))
                i++ ;
            else
              while (i < strlen(*ps) && inset((*ps)[i],"azAZ09__"))
                i++ ;
            res = txtSub(*ps,0,i) ;
            (*ps) += i ; }
          else {
            if (txtPos("/*-+=&%><",txtC((*ps)[0]))<999) {
              i = 1 ;
              if ((strlen(*ps) >=2) && txtPos("/*-+=&%><",txtC((*ps)[1])) <999 ) {
                i = 2 ;
                if ((strlen(*ps) > 2) && ((*ps)[2] == '='))
                  i = 3  ;}
              res = txtSub(*ps,0,i) ;
              (*ps)+=i  ; }
            else {
              res = txtSub(*ps,0,1) ;
              (*ps)++ ; } } }  }  } }
  if (txtPos(res,"/*") == 0 || txtPos(res,"//") == 0 || commenteol || commentmode)
    res = txtEmpty ;
  return res ;
}

tXt LoopMacro = "" ;
tXt EvStartMacro = "" ;
static int loopadd = 1 ;

tXt continStr(int * num) {
  (*num)++ ;
  if (*num >= 3) {
    *num = 0 ;
    return txtConcat("\\\n",NULL) ; }
  else
    return txtEmpty ; }
 
void updateLoopMacro(tXt name,int rise) {
  if (!LoopMacro[0]) 
    LoopMacro = fridge("#define EvLoopMacro") ;
  int p = txtPos(LoopMacro,txtConcat("(",name,")",NULL)) ;
  if (p > 999) {
    LoopMacro = txtConcat(unfridge(LoopMacro),continStr(&loopadd),NULL) ;
    if (rise == 2)
      LoopMacro = fridge(txtPrintf("%s checkChange(%s) ;",LoopMacro,name)) ;
    else
      LoopMacro = fridge(txtPrintf("%s checkinp%c(%s) ;",LoopMacro,(rise?'h':'l'),name)) ;}
  else {
    char hl = LoopMacro[p-1] ;
    if (hl != ' ' && hl != 'e')
      if ((rise == 1) != (hl == 'h'))
        txtPoke(LoopMacro,p-1,' ') ;}
 if (txtAnyError())
    printf("Err1: %s\n",txtLastError()) ;

}

tXt getext(tXt s) {
  char *p = strstr(s,".") ;
  if (!p)
    return txtEmpty ;
  else {
    ++p ;
    char * p2 = strstr(p,".") ;
    while (p2) {
      p = ++p2 ;
      p2 = strstr(p,".") ; }
    return txtSub(p,0,99) ; }
}
 
tXt unComment(tXt s) {
  int p = txtPos(s,"//") ;
  if (p > 999)
    p = txtPos(s,"/*") ;
  if (p > 999)
    return s ;
  else
    return txtSub(s,0,p-1) ; }
   
tXt unRightBracket(tXt s) {
  s = txtTrim(s) ;
  if (s[strlen(s)-1] == '}' && txtPos(s,"{") >= 999)
    s = txtSub(s,0,strlen(s)-1) ;
  return s ; }
  
tXt prevtoken[8] = {" "," "," "," "," "," "," "} ;

bool tokens (tXt a, tXt b, tXt c,tXt d) {
  int i ;
  tXt abcd[4] = {d,c,b,a} ;
  forloop(i,0,3)
    if (abcd[i][0])
      if (!EQS(abcd[i],prevtoken[i])) {
        i = 999 ; }
  return i < 10 ; }



bool harvestEnabled = true ;
tXt defines = "" ;    


tXt txthalfSplit(tXt s,char c,int h) {
  int p = txtPos(s,txtC(c)) ;
  if (h)
    return txtSub(s,p+1,999) ;
  else
    return txtSub(s,0,p) ; }


void includetest(void) {
  tXt s ;
  FILE * stack[3] ;
  int stackp = 0 ;
  tXt finame = txtConcat("test.ino",NULL) ;
  while (readstringfrom(finame,&s,&fglob1) || stackp > 0)  {
    if (stackp > 0 && fglob1 == NULL ) {
      fglob1 = stack[--stackp] ; }
    if (txtPos(s,"#include ") == 0 ) {
      stack[stackp++] = fglob1 ;
      fglob1 = NULL ;
      finame = txthalfSplit(s,' ',1) ; }
    printf("%s\n",s) ; }
 }



bool ifstack[20] ;
int ifstackp = 0 ;

void setIf(bool enab) {
  if (ifstackp < 20) 
    ifstack[ifstackp++] = harvestEnabled ;
  harvestEnabled = enab ; }
  
bool popEnabled(void) {
  if (ifstackp > 0)
    return ifstack[--ifstackp] ;
  else 
    return false ;
}

int txtLastPos(tXt s , tXt zk) {
  int rslt = txtNOTFOUND ;
  const char * p = s ;
  const char * q ;
  int rep = 0 ;
  while (rep++ < 1000 && (q = strstr(p,zk)) != NULL) {
    rslt = q-s ;
    p = q+1 ; }
  return rslt ;
  }

int txtHwPos(tXt s , tXt zk) {
  int zlen = strlen(zk) ;
  int p = txtPos(s,zk) ;
  if (p > 999)
    return p ;
  char cbev = ' ' ;
  if (p > 0)
    cbev = s[p-1] ; 
  if (!inset(cbev,"azAz09__")  && !inset(s[p+zlen],"azAZ09__"))
    return p ;
  else
    return p+zlen+txtHwPos(&s[p+zlen],zk) ; }

tXt protostring ;
bool protoloading = false ;
bool hasNextLabels = false ;
int bracklevel = 0 ;
bool rpimode = false ;
bool defineline = false ;
int main(int argc, char * argv[])  {
  tXt finame = "" ;
  tXt foname = "" ;
  tXt hcopy = "" ;
  tXt path = "" ;
  protostring = txtEmpty ;
  int linecount = 0 ;
  int charcount = 0 ;
  //print3int(txtLastPos("abcdgfhabcxyz","bc"),0,0) ;
  int i ;
  int li ;
  tXt s,tkn ;
  FILE * fstack[3] ;
  int fstackp = 0 ;
  tXt globs = fridge(";") ;
  LoopMacro = fridge("#define EvLoopMacro") ;
  EvStartMacro = fridge("#define EvStartMacro ") ;
  defines = fridge(";") ;
  int startadd = 0 ;
  if (argc >=2 && EQS(getext(argv[1]),"ino")) {
    finame = fridge(argv[1]) ;
    foname = fridge("harvested.h") ; }
  else {
    forloop(i,1,argc-1) {
      if (EQS(argv[i],"rpi") && i < argc-1) {
        rpimode = true ;
        if (EQS(getext(argv[i+1]),"c"))
          finame = fridge(txtConcat(argv[i+1],NULL)) ; }
      if (EQS(argv[i],"charvest") && i < argc-1)
        if (EQS(getext(argv[i+1]),"c"))
          finame = fridge(txtConcat(argv[i+1],NULL)) ;
      if (EQS(getext(argv[i]),"cpp")) 
        if (txtPos(argv[i],"\\sketch\\") < 999)
          finame = fridge(txtPrintf("%s",argv[i])) ;}
    if (!finame[0]) {
      printf("no valid cpp file in arguments\n") ;
      return 0 ; }
    if (txtPos(finame,"\\") > 999)
      path = fridge(".") ;
    else  
      path = fridge(txtSub(finame,0,txtLastPos(finame,"\\"))) ;
    foname = fridge(txtConcat(path,"\\harvested.h",NULL)) ;}
  print3string(foname,finame," ") ;
  while (  readstringfrom(finame,&s,&fglob1) || fstackp > 0) {
    linecount++ ;
    charcount += strlen(s)+2 ;
    if (fglob1 == NULL) {
      fglob1 = fstack[--fstackp] ;
      s = "" ; }
    if (txtPos(s,"//gBl") == 0)
      writestringto(foname,txtSub(s,5,999),&fglob2) ;
    tXt slin = txtConcat(s,NULL) ;  
    int rep = 0 ;
    while (s[0] ) {
      tXt tkn = getctoken(&s) ;
      printf("tkn %s\n",tkn) ;
      if (tkn[0]) { 
        intloop(i,0,6)
          prevtoken[7-i] = prevtoken[6-i] ;
        prevtoken[0] = txtConcat(tkn,NULL) ;
        //printf("%c->%s\n",defineline?'d':'n',tkn) ;
        if (tokens("","","#","define") )
          defineline = true ;
        if (EQS(tkn,"}")) {
          if (--bracklevel < 0) {
            if (!defineline)
              fprintf(stderr,"too many \'}\' %s\n",slin ) ; } }
        if (EQS(tkn, "{"))
          bracklevel++ ; 
        if (protoloading) {          
          if (EQS(tkn, "{")) {
            protoloading = false ;
            writestringto(foname,txtConcat(protostring," ;",NULL),&fglob2) ;
            protostring = txtEmpty ; }
          else
            protostring = txtConcat(protostring,txtRepeat(" ",spacecnt),tkn,NULL) ; } 
        if (EQS(tkn,"pRoTo") && !defineline) {
          protoloading = true ; }
        if (tokens("","#","define","")) {
          if (tokens("scanharvest","","",""))
            defines = fridge(txtConcat(unfridge(defines),prevtoken[0],";",NULL)) ;
          //defineline = true ; 
          }
        if (tokens("scanharvest","#","undef","")) 
          defines = fridge(txtReplace(unfridge(defines),txtConcat(";",prevtoken[0],";",NULL),";")) ;
        if (tokens("scanharvest","#","ifdef","") )
          setIf(txtPos(defines,txtConcat(";",prevtoken[0],";",NULL)) != txtNOTFOUND) ;
        if (tokens("scanharvest","#","ifndef","") )
          setIf(txtPos(defines,txtConcat(";",prevtoken[0],";",NULL)) == txtNOTFOUND) ;
        if (tokens("","scanharvest","#","else") )
          harvestEnabled = !harvestEnabled ;
        if (tokens("","scanharvest","#","endif") ) {
          //writestringto(foname,txtPrintf("//endif"),&fglob2) ;
          harvestEnabled = popEnabled() ; }
        if (tokens("scanharvest","#","","") )
          writestringto(foname,txtPrintf("// %d %s %s %s %s",harvestEnabled
          ,prevtoken[3],prevtoken[2],prevtoken[1],prevtoken[0]),&fglob2) ; 
          
        if (harvestEnabled) {
          if (tokens("scanharvest","#","include","") )
            if (fstackp < 2) {
              fstack[fstackp++] = fglob1 ;
              fglob1 = NULL ;
              unfridge(finame) ;
              finame = fridge(txtConcat(path,"\\",txtTrim(txtReplace(prevtoken[0],"\"","")),NULL)) ;
              writestringto(foname,txtPrintf("//including %s",finame),&fglob2) ;}
          if (!tokens("#","define","","")) {
              if (txtPos(prevtoken[1], "glob_") == 0 && isalpha(tkn[0])) {
                if (txtPos(globs,txtPrintf(";%s;",tkn)) < 999)
                  fprintf(stderr,"Error glob_ %s redefined\n",tkn) ;
                else  
                  globs = fridge(txtConcat(unfridge(globs),tkn,";",NULL)) ;
                //printf("%s %s %s\n",prevtoken[2],prevtoken[1],tkn) ;
                writestringto(foname,txtConcat(txtSub(prevtoken[1],5,99)," ",tkn, " ;",NULL),&fglob2) ;} } 
          if (tokens("","#","define","harvestedcopy")) {
            hcopy = fridge(txtTrim(s)) ;}
          if (tokens("","#","define","hasNextLabels")) {
            hasNextLabels = true ;}
          if (tokens("starterFunct","","(","void")) {
            /*if (++startadd >= 3) {
              startadd = 0 ;
              EvStartMacro = fridge(txtConcat(unfridge(EvStartMacro),"\\\n",NULL)) ;}*/
            EvStartMacro = fridge(txtPrintf("%s%s %s();",unfridge(EvStartMacro),continStr(&startadd),prevtoken[2])) ;}
          if (EQS(prevtoken[0],"{") && txtPos(prevtoken[1],"EH_") == 0) {
            if (bracklevel != 1)
              fprintf(stderr,"bracket level %d at %s\n",bracklevel,prevtoken[1]) ;
            writestringto(foname,txtConcat(
            "#define ",prevtoken[1]," void ",txtSub(prevtoken[1],3,999),"(void)",NULL)
            ,&fglob2) ;}
    
          if (!EQS(prevtoken[4],"define") ) {
            if (tokens("onRise","(","", ")")) 
              updateLoopMacro(prevtoken[1],1) ;
            if (tokens("onFall","(","", ")")) 
              updateLoopMacro(prevtoken[1],0) ;
            if (tokens("onChange","(","",")"))
              updateLoopMacro(prevtoken[1],2) ; }
          if (EQS(tkn,"addSetup")  && !tokens("","#","define","") ) {
            EvStartMacro = fridge(txtConcat(unfridge(EvStartMacro),continStr(&startadd)," ",unRightBracket(unComment(s))
            ,NULL)) ; 
            }
          if (EQS(tkn,"addLoop")  && !tokens("","#","define","") ) {
            LoopMacro = fridge(txtConcat(unfridge(LoopMacro),continStr(&loopadd)," ",unRightBracket(unComment(s))
            ,NULL)) ; 
            } } } } }
  if (EvStartMacro[0])
    writestringto(foname,unfridge(EvStartMacro),&fglob2) ; 
  if (LoopMacro[0])
    writestringto(foname,unfridge(LoopMacro),&fglob2) ;
  finishfile(&fglob2) ;
  finishfile(&fglob1) ;
  if (hcopy[0]) {
    writestringto(hcopy,txtPrintf("//foname: %s",foname),&fglob2) ;
    while (readstringfrom(foname,&s,&fglob1) ) {
      writestringto(hcopy,s,&fglob2) ;}
    writestringto(hcopy,txtConcat("//defines: ",defines,NULL),&fglob2) ;
    if (txtAnyError())
      writestringto(hcopy,txtPrintf("//text error %s",txtLastError()),&fglob2) ;
    finishfile(&fglob2) ; } 
  if (txtAnyError())
    printf("Err: %s\n",txtLastError()) ;
  /* foname = fridge(txtConcat(path,"\\even.tmp",NULL)) ;  
  while (readstringfrom(finame,&s,&fglob1) )
    writestringto(foname,s,&fglob2) ; */
  finishfile(&fglob1) ;  
  finishfile(&fglob2) ;
  //int n = rename(finame,txtConcat(path,"\\effencpy.tmp",NULL) ) ;
  if (hasNextLabels) {
    tXt src = fridge(txtConcat(path,"\\effen.tmp",NULL)) ;
    remove(src) ;
    int n = rename(finame,src)  ;
    if (n == 0) {
      int nextnr = 1 ;
      while (readstringfrom(src,&s,&fglob1)) {
        int p = txtHwPos(s,"next") ;
        if (p < 999) {
          s = txtInsert(s,txtPrintf("_%d",nextnr),p+4) ;
          if (txtPos(s,"void ") == 0 )
            nextnr++ ; }
        p = txtPos(s," vdelay(") ;
        if (p < 999 && txtPos(s,"#define") >999) {
          s = txtPrintf("%s later(nd,next_%d);} void next_%d(void) {",s,nextnr,nextnr) ;
          nextnr++ ; }
        p = txtPos(s," waitfor(") ;
        if (p < 999 && txtPos(s,"bool") >=999) {
          tXt s1 = txtTrim(s) ;
          s1 = txtReplace(txtSub(s1,0,strlen(s1)-1),"waitfor(","sleepWait(") ;
          s = txtPrintf("later(0,next_%d);} void next_%d(void) { if(%s) return ;",nextnr,nextnr,s1) ;
          nextnr++ ; }
        p = txtHwPos(s,"EH_next") ;
        if (p < 999) 
          s = txtInsert(s,txtPrintf("_%d",nextnr++),p+7) ;
        writestringto(finame,s,&fglob2) ;}
      finishfile(&fglob2) ;} }
  FILE * flog = fopen("\\micros\\harv.log","a+") ;
  fprintf(flog,"ticks: %d\n",GetTickCount()) ;
  fprintf(flog,"lines %d chars %d file: %s\n",linecount,charcount,finame) ;
  fclose(flog) ;
  return 1 ;
}  

