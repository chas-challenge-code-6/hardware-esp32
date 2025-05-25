#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

// Testing web server, will give http response code 200 on anything
// and print request on stdout.

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    const int PORT = 8080;
    char buffer[4096];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on port " << PORT << std::endl;

    while (true)
    {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            continue;
        }
        std::string request;
        ssize_t valread = 0;
        bool headers_done = false;
        size_t content_length = 0;

        while (!headers_done && (valread = read(client_fd, buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[valread] = '\0';
            request += buffer;
            size_t pos = request.find("\r\n\r\n");
            if (pos != std::string::npos)
            {
                headers_done = true;
                size_t cl_pos = request.find("Content-Length:");
                if (cl_pos != std::string::npos)
                {
                    size_t cl_end = request.find("\r\n", cl_pos);
                    std::string cl_line = request.substr(cl_pos, cl_end - cl_pos);
                    content_length = std::stoi(cl_line.substr(15));
                    size_t body_start = pos + 4;
                    size_t body_have = request.size() - body_start;
                    while (body_have < content_length)
                    {
                        valread = read(client_fd, buffer,
                                       std::min(sizeof(buffer) - 1, content_length - body_have));
                        if (valread <= 0)
                            break;
                        buffer[valread] = '\0';
                        request += buffer;
                        body_have += valread;
                    }
                }
            }
        }
        std::cout << "--- Client Request ---\n"
                  << request << "\n----------------------" << std::endl;
        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
                               "2\r\nConnection: close\r\n\r\nOK";
        send(client_fd, response, strlen(response), 0);
        close(client_fd);
    }
    close(server_fd);
    return 0;
}