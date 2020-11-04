#include <stdio.h> 
#include <stdlib.h> 
#include <sys/time.h>
#include <time.h>
#include <unistd.h> 
#include <signal.h>
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <errno.h>

#define MAXLINE 2048 

#define CLIENT_IP_LEN 128
#define IFACE_NAME_LEN 64

static unsigned int uiPacketsReceived=0;
static unsigned int uiPacketsSent=0;
static unsigned int uiBytesReceived=0;
static unsigned int uiBytesSent=0;
static struct timeval lTimeFirstPacketReceived;
static struct timeval lTimeLastPacketReceived;
static unsigned char ucFirstFlag=0;
/*
static char iface[IFACE_NAME_LEN];
int get_local_ip(const char *eth_inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

    close(sd);
    return 0;
}
*/
static char accept_client_ip[CLIENT_IP_LEN];
static unsigned short local_port=3479;
static char *local_ip = "0.0.0.0";


int checkTheClientIP(char *clientIp)
{
    if(strcmp(clientIp,accept_client_ip)==0)
    	return 1;

    return 0;
}

static void convertTimevalToSystime(struct timeval tv, char *systm)
{
    struct tm *t;
    t = localtime(&tv.tv_sec);
    sprintf(systm,"%d-%d-%dT%d:%d:%d.%ld", 1900+t->tm_year, 1+t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec);
}

static writeUdpechoResult()
{
        FILE *fp=fopen("/tmp/udpecho_result","w");
        if(fp)
        {
            char buf[256]={0};
            char fTm[32]={0};
            char lTm[32]={0};

            convertTimevalToSystime(lTimeFirstPacketReceived,fTm);
            convertTimevalToSystime(lTimeLastPacketReceived,lTm);
            sprintf(buf,"%d %d %d %d %s %s",uiPacketsReceived,uiPacketsSent,uiBytesReceived,uiBytesSent,fTm,lTm);
            fputs(buf,fp);
            fclose(fp);
        }

}


void loop_echo(int sockfd, struct sockaddr_in *cliaddr, socklen_t clilen)
{
    int n;
    char buffer[MAXLINE];
    socklen_t len;
    char clientIp[INET_ADDRSTRLEN];

    uiPacketsReceived=0;
    uiPacketsSent=0;
    uiBytesReceived=0;
    uiBytesSent=0;

    for(; ;)
    {
    	len = clilen;  //len is value/resuslt 
	struct sockaddr *client = (struct sockaddr *)cliaddr;
    	n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, client, &len); 
    	buffer[n] = '\0'; 
	inet_ntop(AF_INET, &(cliaddr->sin_addr), clientIp, INET_ADDRSTRLEN);
    	printf("Client : %s %s\n", buffer,clientIp);
    	if(checkTheClientIP(clientIp)==1)
	{
	    if(ucFirstFlag==0)
	    {
		gettimeofday(&lTimeFirstPacketReceived,NULL);
		ucFirstFlag=1;
            }
	    else
	    {
		gettimeofday(&lTimeLastPacketReceived,NULL);
            }

	    uiPacketsReceived++;
	    uiBytesReceived+=n;
	    if(sendto(sockfd, buffer, n, MSG_CONFIRM, client,len)>0)
	    { 
	    	uiPacketsSent++;
            	uiBytesSent+=n;
	    }
	    writeUdpechoResult();
	    system("/usr/share/easycwmp/functions/tr143_udpecho_launch set");
	}
	else
	{
	}
    }
}

static void usage(char *name)
{
    fprintf(stderr, "Usage: udpecho_server [-l local ip] [-c client ip] [-p local port]\n");
}

static void sigHandle(int signo)
{
    //show the info to file
    if(signo == SIGUSR1)
    {
	writeUdpechoResult();
    }
}

int main(int argc, char *argv[]) { 
    int opt;
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 

    while ((opt = getopt(argc, argv, "hl:c:p:")) != -1) {
        switch (opt) {
	    case 'l':
                local_ip=optarg;
		break;
	    case 'c':
		memset(accept_client_ip,0,sizeof(char)*CLIENT_IP_LEN);
            	strcpy(accept_client_ip,optarg);
            	break;
            case 'p':
            	local_port = atoi(optarg);
            	break;
            case 'h':
            default:
            	usage(argv[0]);
         	return -1;
	}
    }


    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    //int reuse_addr = SO_REUSEADDR;
    //setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
     
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    //servaddr.sin_addr.s_addr = INADDR_ANY; 
    if(inet_pton(AF_INET, local_ip, &(servaddr.sin_addr))!=1)
    {
	fprintf(stderr,"local ip failed: %s\n",local_ip);
	close(sockfd);
	exit(EXIT_FAILURE);
    }
    servaddr.sin_port = htons(local_port); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed");
	close(sockfd);	
        exit(EXIT_FAILURE); 
    } 
 
    signal(SIGUSR1, sigHandle);     
    loop_echo(sockfd, &cliaddr, sizeof(cliaddr));
    return 0;
} 

