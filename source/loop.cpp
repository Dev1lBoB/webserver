//
// Created by Pamula File on 5/8/21.
//
#include "../header.h"
#include "Number.hpp"

int		sorter(const Header& a, const Header& b)
{
	return (a.getClient() < b.getClient());
}

std::string receive_buffer( std::list<Header>::iterator &it, server &serv )
{
    int         z;
    char        buf[BUFSIZE + 1];
    std::string ret;

    z = recv( it->getClient(), buf, BUFSIZE, 0 );
    if (z == -1)
    {
        if (errno == EAGAIN)
            return std::string("wait");
        else
            return std::string("error");
    }
    else if (z == 0)
        return std::string("connection closed");
    else
    {
        buf[z] = '\0';
        it->setBuffer(it->getReminder() + buf);
        it->setReminder(std::string());
        return split_buffer( it->getBuffer(), *it, serv );
    }
}

int receive( std::list<Header>::iterator &it, server &serv, fd_set *clients_with_data )
{
	if ( FD_ISSET(it->getClient(), clients_with_data) and !it->isBodyEnd())
	{
        std::string str;

        str = receive_buffer( it, serv );
        if (str == "connection closed")
            return 1;
	}
    return 0;
}
#ifndef BONUS
void sendFile( Header &head, config &conf )
{
	static char    str[BUFSIZE + 1];
	size_t  z = 0;
    int fd = head.getFile();
    size_t  diff = BUFSIZE - head.getReminder().length();
    struct stat st;

    bzero(str, sizeof(str));
    ::fstat(fd, &st);
    if (st.st_size == 0 || diff == 0 || head.getMethod() == "HEAD" ||  head.getMethod() == "DELETE")
        send_protected("", head);
    else if ((z = read(fd, str, diff)) > 0)
	{
		str[z] = 0;
        if ( send_protected(str, head))
            return ;
	}
    if (z != diff and head.getReminder().empty())
        update_descriptors( head.getRealPathToFile( ), head, conf );
}
#endif
#ifdef BONUS
void sendFile( Header &head, config &conf )
{
	if (head.write_management and !head.management_fd)
	{
		head.management_fd = open((head.getRealPathToFile() + ".management").c_str(), O_WRONLY | O_CREAT, 0777);
		if (head.management_fd == -1)
			error_exit("no fd");
	}
	static char    str[BUFSIZE + 1];
	size_t  z = 0;
    int fd = head.getFile();
    size_t  diff = BUFSIZE - head.getReminder().length();
    struct stat st;

    bzero(str, sizeof(str));
        ::fstat(fd, &st);
    if (head.use_management)
    {
        if ((z = read(fd, str, diff)) > 0)
        {
            str[z] = 0;
            if ( send_protected(str, head))
                return ;
        }
        if (!z && !head.getReminder().empty())
            send_protected("", head);
        if (z != diff and head.getReminder().empty())
            update_descriptors( head.getRealPathToFile( ), head, conf );
    }
    else
    {
        if (st.st_size == 0 || diff == 0 || head.getMethod() == "HEAD" ||  head.getMethod() == "DELETE" )
            send_protected("", head, head.management_fd);
        else if ((z = read(fd, str, diff)) > 0)
        {
            str[z] = 0;
            if ( send_protected(str, head, head.management_fd))
                return ;
        }
        if (z != diff and head.getReminder().empty())
        {
            update_descriptors( head.getRealPathToFile( ), head, conf );
            if (head.management_fd)
                close(head.management_fd);
        }
    }
}
#endif

void buildHeader( Header &head )
{
	std::string str;

	if (head.getMethod() == "HEAD")
	    head.setContent_Length("0");
	str = head.getResponse();
	if (!head.getIsCgi())
	    str += head.getContent_Length();
	else
	{
	    head.setTransfer_Encoding("Transfer-Encoding: chunked");
        str += "Transfer-Encoding: " + head.getTransfer_Encoding() + END;
    }
    str += head.getContent_Language();
    str += head.getAllow();
	str += head.getDate();
	str += head.getLast_Modified();
	str += head.getContent_Location();
	str += head.getHostHeaderResponse();
	str += END;
	head.setEmptyLine(false);
	head.setReminder(str);
}

int     parse_cgi(Header &head, char *line)
{
	char *tmp;

	if ((tmp = strstr(line, "Status: ")))
	{
		tmp += strlen("Status: ");
		head.setResponse("HTTP/1.1 " + std::string(tmp));
	}
	else if ((tmp = strstr(line, "Content-Type: ")))
		head.setContent_Type(std::string(tmp));
	else
        return 1;
    return 0;
}

std::string getBaseSixteen(unsigned int n)
{
	std::string str("");
	unsigned int num = n;
	int c = 1;

	while (num / 16)
	{
		num /= 16;
		c *= 16;
	}
	while (c != 0)
	{
		str += n / c < 10 ? n / c + 48 : n / c + 22 + 'A';
		n %= c;
		c /= 16;
	}
	return str;
}

#ifndef BONUS
void sendFileChunked( int fd, config &conf, Header &head )
{
	static char line[BUFSIZE + 1];
	std::string str;
	int z;

	if ((head.getMethod() == "POST" or head.getMethod() == "PUT") and !head.getIsCgi())
    {
        send_protected("", head);
        update_descriptors(  head.getRealPathToFile( ),  head, conf );
    }
	else
    {
	    if (head.getReminder().length() >= BUFSIZE)
        {
            send_protected("", head);
        }
	    else
        {
            z = read(fd, line, BUFSIZE - 9 - head.getReminder().length());
            if (z == 0)
            {
                if (head.getReminder().length())
                {
                    send_protected("", head);
                    if (head.getReminder().length())
                        return ;
                }
                send_protected("0\r\n\r\n",  head );
                update_descriptors(  head.getRealPathToFile( ),  head, conf );
                return;
            }
            line[z] = 0;
            str = (getBaseSixteen(z) + "\r\n" + line + "\r\n");
            send_protected(str,  head );
        }
    }
}
#endif
#ifdef BONUS
void sendFileChunked( int fd, config &conf, Header &head )
{
    static char line[BUFSIZE + 1];
    std::string str;
    int z;

    if (head.write_management and !head.management_fd)
    {
        head.management_fd = open((head.getRealPathToFile() + ".management").c_str(), O_WRONLY | O_CREAT, 0777);
        if (head.management_fd == -1)
            error_exit("no fd");
    }
    if ((head.getMethod() == "POST" or head.getMethod() == "PUT") and !head.getIsCgi())
    {
        send_protected("", head);
        update_descriptors(  head.getRealPathToFile( ),  head, conf );
    }
    else
    {
        if (head.getReminder().length() >= BUFSIZE)
        {
            send_protected("", head, head.management_fd);
        }
        else
        {
            z = read(fd, line, BUFSIZE - 9 - head.getReminder().length());
            if (z == 0)
            {
                if (head.getReminder().length())
                {
                    send_protected("", head, head.management_fd);
                    if (head.getReminder().length())
                        return ;
                }
                send_protected("0\r\n\r\n",  head, head.management_fd );
                update_descriptors(  head.getRealPathToFile( ),  head, conf );
                return;
            }
            line[z] = 0;
            str = (getBaseSixteen(z) + "\r\n" + line + "\r\n");
            send_protected(str,  head, head.management_fd );
        }
    }
}
#endif

static int Select( int max_fd, config &conf )
{
    int ret;
    if ((ret = select( max_fd, &conf.conf_set, &conf.write_set, NULL, &conf.tv)) < 0)
    {
        if (errno != EINTR)
        {
            perror("EINTR");
            return 1;
        }
        else
        {
            perror("errno");
            return 1;
        }
    }
    if (!ret)
    {
        return 1;
    }
	usleep(conf.sockets.size() * 1000);
	return (0);
}

void    loop(config &conf)
{
    std::list<Header>::iterator it;
    std::list<Header>::iterator ite;
    std::list<server>::iterator it_serv;
    std::list<server>::iterator ite_serv;
    int                         temp_sock;

    while (true)
    {
        it_serv = conf.getServers().begin();
        ite_serv = conf.getServers().end();
        FD_ZERO(&conf.conf_set);
        conf.sockets.clear();
        while (it_serv != ite_serv)
        {
            temp_sock = it_serv->getHostSock( );
            FD_SET( temp_sock, &conf.conf_set );
            conf.sockets.push_back(temp_sock);
            it = it_serv->getSet().begin();
            ite = it_serv->getSet().end();
            while ( it != ite)
            {
                temp_sock = it->getClient();
                FD_SET(temp_sock, &conf.conf_set);
                conf.sockets.push_back(temp_sock);
                if (it->getFile())
                    FD_SET(temp_sock, &conf.write_set);
                it++;
            }
            it_serv++;
        }
        conf.sockets.sort();
        if ( Select( conf.sockets.back() + 1, conf ))
            continue ;
        it_serv = conf.getServers().begin();
        ite_serv = conf.getServers().end();
        while (it_serv != ite_serv)
        {
            if (FD_ISSET( it_serv->getHostSock( ), &conf.conf_set))
            {
                it_serv->getSet().push_back( Header( *it_serv, conf.getEnv()));
            }
            communication_with_clients( it_serv->getSet( ), *it_serv, &conf.conf_set, conf );
            it_serv++;
        }
    }
}
