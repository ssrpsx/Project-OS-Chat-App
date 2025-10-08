#include "header.h"

std::mutex mtx;
std::vector<std::string> client_queues;

void handle_register(const std::string &msg)
{
    std::string qname = msg.substr(9);
    std::lock_guard<std::mutex> lock(mtx);
    client_queues.push_back(qname);

    std::cout << "\033[32m" << qname << " has join the server!\033[0m" << std::endl;
}

void broadcast(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(mtx);

    int pos1 = msg.find('[');
    std::cout << "\033[35m" << "Message sent by " << msg.substr(0, pos1) << "\033[0m" << std::endl;
    for(const auto &qname : client_queues)
    {
        mqd_t client_q = mq_open(qname.c_str(), O_WRONLY);

        if(client_q != (mqd_t)-1)
        {
            mq_send(client_q, msg.c_str(), msg.size() + 1, 0);
            mq_close(client_q);
        }
    }
}

int main()
{
    mq_unlink(CONTROL_Q);

    struct mq_attr attr;
    attr.mq_flags = 0;                 // flag เช่น non-blocking
    attr.mq_maxmsg = 10;               // จำนวน message สูงสุดใน queue
    attr.mq_msgsize = size_of_message; // ขนาด message สูงสุด bytes
    attr.mq_curmsgs = 0;               // จำนวน message ปัจจุบันใน queue

    mqd_t server_q = mq_open(CONTROL_Q, O_CREAT | O_RDWR, 0666, &attr);
    if (server_q == (mqd_t)-1)
    {
        perror("mq_open server");
        return 1;
    }

    std::cout << "\033[32m" << "Server opened" << "\033[0m" << std::endl;
    char buffer[size_of_message];

    while (true)
    {
        ssize_t bytes = mq_receive(server_q, buffer, sizeof(buffer), nullptr);
        if (bytes > 0)
        {
            buffer[bytes] = '\0';
            std::string msg(buffer);

            if (msg.find("REGISTER:") == 0)
            {
                std::thread t(handle_register, msg);
                t.detach();
            }
            else if(msg.find("[SAY]:"))
            {
                broadcast(msg);
            }
        }
        
    }

    mq_close(server_q);
    mq_unlink(CONTROL_Q);
    return 0;
}