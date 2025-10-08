#include "header.h"

void listen_queue(const std::string &qname, const std::string &name)
{
    mqd_t client_q = mq_open(qname.c_str(), O_RDONLY);
    if (client_q == (mqd_t)-1)
    {
        perror("mq_open client listen");
        return;
    }

    char buffer[size_of_message];
    while (true)
    {
        ssize_t bytes = mq_receive(client_q, buffer, sizeof(buffer), nullptr);

        if (bytes > 0)
        {
            buffer[bytes] = '\0';

            // form PHAI[SAY]: something
            std::string msg(buffer);
            int pos1 = msg.find('[');
            int pos2 = msg.find(']');

            if (name != msg.substr(0, pos1))
            {
                std::cout << "\033[32m" << msg << "\033[0m" << std::endl << "> " << std::flush;
            }
        }
    }

    mq_close(client_q);
}

int main()
{   
    std::string client_name;
    std::string client_qname;

    std::cout << "\nEnter your ChatName: ";

    std::cin >> client_name;
    client_qname = "/client_" + client_name;

    // สร้าง queue เพื่อ client เอาไป register
    struct mq_attr attr;
    attr.mq_flags = 0;                 // flag เช่น non-blocking
    attr.mq_maxmsg = 10;               // จำนวน message สูงสุดใน queue
    attr.mq_msgsize = size_of_message; // ขนาด message สูงสุด bytes
    attr.mq_curmsgs = 0;               // จำนวน message ปัจจุบันใน queue

    mqd_t client_q = mq_open(client_qname.c_str(), O_CREAT | O_RDONLY, 0666, &attr);

    if (client_q == (mqd_t)-1)
    {
        perror("mq_open client");
    }
    mq_close(client_q);
    std::cout << "connected successfully\n" << std::endl;
    std::cout << "-------------------------" << std::endl;
    std::thread t(listen_queue, client_qname, client_name);
    t.detach();

    mqd_t server_q = mq_open(CONTROL_Q, O_WRONLY);
    std::string reg_msg = "REGISTER:" + client_qname;
    mq_send(server_q, reg_msg.c_str(), reg_msg.size() + 1, 0);

    std::cout << "Registered as " << client_name << std::endl;
    std::string msg;

    while (std::getline(std::cin, msg))
    {
        // form PHAI[SAY]: something
        if (msg.find("SAY:") == 0)
        {
            std::string send_msg = client_name + "[SAY]:" + msg.substr(4);
            mq_send(server_q, send_msg.c_str(), send_msg.size() + 1, 0);
        }
        else if (msg.find("JOIN:") == 0)
        {
            std::string send_msg = client_name + "[JOIN]:" + msg.substr(5);
            mq_send(server_q, send_msg.c_str(), send_msg.size() + 1, 0);
        }
        std::cout << "> " << std::flush;
    }
    
    mq_close(server_q);
    mq_unlink(CONTROL_Q);
    return 0;
}