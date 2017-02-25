All HelloX supported SDK,include the standard C Runtime Library,OS API,and other APIs are put
into this directory.
Principles when port C Runtime Library:
1. This CRT is apply for application development,it may not same as the one in kernel;
2. All OS offered APIs are put into kapi.h/kapi.c files;
3. CPU endian is defined in kapi.h file;


Main revision than in kernel:
1. printf is not thread safe;
2. New system calls should be added(GotoHome,ChangeLine,GetCurrentPos,SetCurrentPos,GetSystemTime);
3. pthread and io related are removed;
4. rand.c file and rand() routine related definitions are put into project;
5. pthread_xxx related files are commented off from project;
6. math.h file is updated accordingly;
7. cpu.h file is added in sys/ and included in kapi.h;
8. 

Issues:
1. log function has error,so comment off and will implement later;
