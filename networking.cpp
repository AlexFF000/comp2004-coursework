/*
    Alex Redmond, Group R
    This is mostly just a modified version of the code from the DynamicWebPageExample from the COMP2004 example repository: https://github.com/UniversityOfPlymouthComputing/COMP2004-C1W2-2021

    Code for running web server for users to access latest sample
*/

#include "networking.h"
#include "Buffer.h"
#include "time.h"

#include "uop_msb_2_0_0.h"
using namespace uop_msb_200;

#include <iostream>
using namespace std;

#include "EthernetInterface.h"
#include "TCPSocket.h"
#include "NTPClient.h"
 
#define HTTP_STATUS_LINE "HTTP/1.0 200 OK"
#define HTTP_HEADER_FIELDS "Content-Type: text/html; charset=utf-8"
#define HTTP_MESSAGE_BODY ""                                     \
"<html>" "\r\n"                                                  \
"  <body style=\"display:flex;text-align:center\">" "\r\n"       \
"    <div style=\"margin:auto\">" "\r\n"                         \
"      <h1>Most Recent Measurements</h1>" "\r\n"                              \
"      <table border=\"1\" style=\"border: 1px solid black; border-collapse: separate;\"><tr><th>Time</th><th>Pressure</th><th>Temperature</th><th>Light</th></tr><tr><td>%s</td><td>%4.3f</td><td>%3.3f</td><td>%3.3f</td></tr></table>" "\r\n"                                 \
"    </div>" "\r\n"                                              \
"  </body>" "\r\n"                                               \
"</html>" "\r\n"
    
#define HTTP_TEMPLATE HTTP_STATUS_LINE "\r\n"   \
                      HTTP_HEADER_FIELDS "\r\n" \
                      "\r\n"                    \
                      HTTP_MESSAGE_BODY "\r\n"
#define HTTP_TITLE     "<head><title> Plymouth Uni Weather Page </title></head>" "\r\n"
#define HTTP_FORMAT_1 "<body style=\"display:flex;text-align:center\">" "\r\n" \
                      "<div style=\"margin:auto\">" "\r\n"
#define HTTP_BOTTOM "</html>" "\r\n"

 
//For the static IP version
#define IP        "10.0.0.10"
#define NETMASK   "255.255.255.0"
#define GATEWAY   "10.0.0.1"

EthernetInterface net;
NTPClient ntp(&net);
LCD_16X2_DISPLAY disp;
DigitalOut lcdBacklight(LCD_BKL_PIN);

int setupEthernet(){
    return net.connect();
}

int runServer(Buffer<readings> *samplesBuffer)
{
    printf("\r\nStarting HTTP Server\r\n");
    
    // Connect the ethernet interface
    //net.set_network(IP, NETMASK, GATEWAY);  //For static IP

    // Get the network address
    SocketAddress a;
    net.get_ip_address(&a);

    // Show the network address
    printf("IP address: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");
    disp.cls();
    disp.printf("%s\n", a.get_ip_address() ? a.get_ip_address() : "None");
    lcdBacklight = 1;

    // Open a TCP socket on the network interface, and create a TCP connection on port 80
    TCPSocket socket;
    socket.open(&net);
    socket.bind(80);

    //Set socket to listening mode (up to 5 connections)
    int err=socket.listen(5);
    if(err==0) {
        printf("Listening OK\n\r");
    }
    else {
        printf("Listen error=%d\n\r",err);
        socket.close();
        while(1);
    }

    while (true)
    {
        disp.locate(1, 0);
        disp.printf("Waiting...      ");

        // ACCEPT Accepting connections
        // .accept() uses wait_any() internally so does not waste CPU time while blocking
        TCPSocket* clt_sock=socket.accept();

        //Unblocks with each connection
        disp.locate(1, 0);
        disp.printf("Connected...    ");

        //Construct web page
        readings mostRecentSample;
        samplesBuffer->readLastN(1, &mostRecentSample);

        char buff[500];
        sprintf(buff, HTTP_TEMPLATE, ::ctime(&mostRecentSample.datetime), mostRecentSample.pressure, mostRecentSample.temperature, mostRecentSample.lightLevel);  //Convert float to string
        
        //Construct response string (in C++)
        string html = string(buff);

        //Send response string (blocking until completed)
        nsapi_size_or_error_t ret = clt_sock->send(html.c_str(), strlen(html.c_str()));  //myHTTP,mydatasize)the rest of this line to use Flash Silicon *see notes above line number 35" myHTTP,strlen(myHTTP));
        

        clt_sock->close();
        disp.locate(1, 0);
        disp.printf("Closed...       ");

        ThisThread::sleep_for(1s);
    }
}

time_t getTime(){
    // Fetch time from NTP server
    ntp.set_server("1.uk.pool.ntp.org", 123);
    return ntp.get_timestamp();
}