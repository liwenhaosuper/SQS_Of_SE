#ifndef SQSCLIENT_H
#define SQSCLIENT_H

#include <vector>
#include <string>


/*!
 *@brief Simple Queue Service Client Class
 *@author liwenhaosuper
 *@date 2012-5-21
 */
class SQSClient{
private:
    //! the master host name
    std::string masterName;
    //! the master port
    int masterPort;
    //! the data node host name
    std::string dataNodeName;
    //! the data node port
    int dataNodePort;
    //! whether the data node is ready
    bool isDataNodeReady;
    //! send http request
    struct evhttp_request* doRequest(std::string dataNode,int port,std::string path);
    //! response call back function
    friend void response_callback(struct evhttp_request *req, void *rsp);
    //! get the data node host and port
    bool getRemoteHost();
public:
    //! constructor
    SQSClient(std::string master,int port):masterName(master),masterPort(port),isDataNodeReady(false){}
    /*!
     *@brief create a queue
     *@param QueueName the queue name to be created
     *@return true if successfully created, otherwise false
     */
    bool CreateQueue(std::string QueueName);
    /*!
     *@brief list all the queue names
     *@return the vector containing all the queue names or
     *      null when fails to connect to data node
     */
    std::vector<std::string> ListQueues();
    /*!
     *@brief delete the queue via its queue name
     *@return true if successfully deleted otherwise false
     */
    bool DeleteQueue(std::string QueueName);
    /*!
     *@brief add a message to a queue
     *@param QueueName the queue name  that the message to be inserted
     *@param Message the message to be inserted
     *@return true of successfully inserted otherwise false
     */
    bool SendMessage(std::string QueueName, std::string Message);
    /*!
     *@brief get a message from a queue
     *@param QueueName the queue name to get the message
     *@param MessageID the message id that belongs to the return message,-1 if no message got
     *@return the message if success, otherwise return ""
     */
    std::string ReceiveMessage(std::string QueueName, int &MessageID);
    /*!
     *@brief delete a message from a queue with a message id
     *@param QueueName the queue name that the message belongs to
     *@param MessageID the message id to be deleted
     *@return true if success , otherwise false
     */
    bool DeleteMessage(std::string QueueName,int MessageID);
};


#endif // SQSCLIENT_H
