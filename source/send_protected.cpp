//
// Created by Pamula File on 6/16/21.
//
#include "header.h"

int send_protected( std::string str, Header &head )
{
    ssize_t  ret;

	str = head.getReminder() + str;
	if (str.length() > BUFSIZE)
    {
	    ret = send(head.getClient(), str.c_str(), BUFSIZE, 0);
    }
	else
        ret = send(head.getClient(), str.c_str(), str.length(), 0);
    if (ret == -1)
    {
        head.setReminder(str);
        return 1;
    }
    else if ((size_t)ret != str.length())
    {
        head.setReminder(str.substr(ret));
        return 1;
    }
    head.setReminder(std::string());
    return 0;
}

int send_protected( std::string str, Header &head, int wfd)
{
	ssize_t  ret;

	str = head.getReminder() + str;
	if (str.length() > BUFSIZE)
	{
		ret = send(head.getClient(), str.c_str(), BUFSIZE, 0);
	}
	else
		ret = send(head.getClient(), str.c_str(), str.length(), 0);
	if (ret == -1)
	{
		head.setReminder(str);
		return 1;
	}
	else if ((size_t)ret != str.length())
	{
	    if (wfd)
		    write(wfd, str.c_str(), ret);
		head.setReminder(str.substr(ret));
		return 1;
	}
    if (wfd)
	    write(wfd, str.c_str(), ret);
	head.setReminder(std::string());
	return 0;
}
