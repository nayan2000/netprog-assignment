// switch(child){
//                     case -1:;
//                         printf(RED"CLIENT SIDE: CHILDREN CAN'T BE CREATED\n"RESET);	
//                         exit(EXIT_FAILURE);

//                     case 0:;
//                         int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
//                         if (r == -1) { perror(0); exit(1); }
//                         if (getppid() == 1)
//                             exit(1);
//                         close(lfd);	
//                         handle_request(cfd);			
//                         _exit(EXIT_SUCCESS);
//                     default:
//                         close(cfd);
//                         break;
//                 }
//             }
//             break;

//             int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
//                 if (r == -1) { perror(0); exit(1); }
//                 if (getppid() == 1)
//                     exit(1);