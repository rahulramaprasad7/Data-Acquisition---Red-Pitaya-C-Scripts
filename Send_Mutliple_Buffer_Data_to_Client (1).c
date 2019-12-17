#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<string.h>
#include<sys/un.h>
#include<unistd.h>
#include"redpitaya/rp.h"
#include"common.h"

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc , char *argv[])
{
	int socket_desc , new_socket , c;
	struct sockaddr_in server , client;
	char *message;
	//char *fileName = "/tmp/output.txt";
	//char *fileName1 = "/tmp/output1.txt";
	//FILE *fp;
	uint32_t start1=0, end1=8191;
    	uint32_t start2=8192, end2=16383;
	int32_t buff_size = 8192;
	int16_t buffer[buff_size+1];
	int16_t buffer2[buff_size+1];
	// int16_t *buffer;
	rp_channel_t channel1=RP_CH_1;
	rp_channel_t channel2=RP_CH_2;

	// Open the File, Check if it Opened
	//fp = fopen(fileName, "w");
	//if (fp == NULL) {
        //	printf("sorry can't open %s\n", fileName);
        //	return 1;
    	//}
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 14000 );
	
	//printf("%d", sizeof(server));
	// Clear the Current Socket

		
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("bindFailed, closing socket...");
		close(socket_desc);
		handle_error("bind");
	}
	puts("bind done");
	
	//Listen
	listen(socket_desc , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (new_socket < 0)
	{
		perror("accept failed");
	}
	
	puts("Connection accepted");
	
	//Reply to the client
	message = "Hello Client , I have received your connection. \n";
	
	// Initialize the Red Pitaya - initialization of API
    	if (rp_Init() != RP_OK) {
        	fprintf(stderr, "Red Pitaya API init failed!\n");
        	return EXIT_FAILURE;
    	}else{
		puts("Initialization successful!\n");
	}
	
	// Iterate while we're reading data
	uint32_t wPos;
	int region = 0;
	
	//Start Acquisition on the Board
	puts("Resetting Acquisition...\n");
	int resetResult = rp_AcqReset();
	if (resetResult == RP_OK){
		puts("reset successful!");
	}else{
		puts("reset failed...");
	}
	
	// Set Decimiation on the Board
	int decimationResult = rp_AcqSetDecimation(1);
	if (decimationResult == RP_OK){
		puts("setting the decimation was successful");
	}else{
		puts("setting the decimation failed");
	}

	// Start Acquistion
    	ECHECK(rp_AcqStart());
	//puts("Checking pointer position...\n");
	//int pointerPos = rp_AcqGetWritePointer(&wPos);
	uint32_t counter = 0;
	int charArraySize = 100;
	while(rp_AcqGetWritePointer(&wPos) == RP_OK){
		//if we're in the first half of the buffer...
		if(region == 0){
			if(wPos > 8191){
				// read Channel 1

				if(rp_AcqGetDataPosRaw(channel1, start1, end1, buffer, &buff_size)!=RP_OK){
                    			fprintf(stderr, "reading 1 failed!\nwPos: %d\nstartPos %d\nendPos %d\nerror Code %d\n",wPos,start1,end1, rp_AcqGetDataPosRaw(channel1, start1, end1, buffer, &buff_size));
                    			return EXIT_FAILURE;
                		}
				if(rp_AcqGetDataPosRaw(channel2, start1, end1, buffer2, &buff_size)!=RP_OK){
                                        fprintf(stderr, "reading 2 failed!\nwPos: %d\nstartPos %d\nendPos %d\nerror Code %d\n",wPos,start1,end1, rp_AcqGetDataPosRaw(channel1, start1, end1, buffer, &buff_size));
                                        return EXIT_FAILURE;
                                }


				// Write the buffer contents to file
				// fprintf(fp,"%s",buffer);
				// fwrite(buffer,sizeof(int16_t),buff_size,fp);
				int i;
        			for(i = 0; i < buff_size; i++){
					char tempStore[charArraySize];
					char tempStore2[charArraySize];
					memset(tempStore,'\0',(sizeof(char) * charArraySize));
					memset(tempStore2,'\0',(sizeof(char) * charArraySize));
                			// printf("%d\n", buffer[i]);
					sprintf(tempStore,"%d\n",(int)buffer[i]);
					sprintf(tempStore,"%d\n",(int)buffer2[i]);
                                        //printf("%s\n",tempStore);
        				write(new_socket , tempStore , charArraySize);
        				write(new_socket , tempStore2 , charArraySize);
				}
				// write(new_socket , buffer , buff_size);

				// the pointer is past the first half, so we can read it
				puts("wPos is larger than 8191, switching region to 1.\n");
				region = 1;
				counter++;
			}
			
		}else{
		// we're in the second half of the buffer...
			if(wPos < 8192){
				// read Channel 1
                                if(rp_AcqGetDataPosRaw(channel1, start1, end1, buffer, &buff_size)!=RP_OK){
                                        fprintf(stderr, "reading 1 failed!\nwPos: %d\nstartPos %d\nendPos %d\nerror Code %d\n",wPos,start1,end1, rp_AcqGetDataPosRaw(channel1, start1, end1, buffer, &buff_size));
                                        return EXIT_FAILURE;
                                }
				if(rp_AcqGetDataPosRaw(channel2, start1, end1, buffer2, &buff_size)!=RP_OK){
                                        fprintf(stderr, "reading 2 failed!\nwPos: %d\nstartPos %d\nendPos %d\nerror Code %d\n",wPos,start1,end1, rp_AcqGetDataPosRaw(channel2, start1, end1, buffer2, &buff_size));
                                        return EXIT_FAILURE;
                                }

                                // Write the buffer contents to file
                                // fwrite(buffer,sizeof(int16_t),buff_size,fp);
				// fprintf(fp,"%s",buffer);
				int i;
                                for(i = 0; i < buff_size; i++){
					char tempStore[charArraySize];
					char tempStore2[charArraySize];
					memset(tempStore,'\0',(sizeof(char) * charArraySize));
					memset(tempStore2,'\0',(sizeof(char) * charArraySize));
                                        // printf("%d\n", buffer[i]);
                           	        sprintf(tempStore,"%d\n",(int)buffer[i]);
                           	        sprintf(tempStore2,"%d\n",(int)buffer2[i]);
					//printf("%s\n",tempStore);
					write(new_socket , tempStore , charArraySize);
					write(new_socket , tempStore2 , charArraySize);
				}
				// write(new_socket , buffer , buff_size);

				// the pointer is back in the first half, so we can read the second half
				puts("wPos is smaller than 8192, switching region to 0.\n");
				region = 0;
				// counter++;
				// if (counter > 1){
				//	puts("Exceeded Counter Limit, returning..");
				//	break;
				// }
			}
		}	
	}
	// write(new_socket , message , strlen(message));
	// Releasing resources
    	rp_Release();
    //	fclose(fp);
	close(socket_desc);
	return 1;
}
