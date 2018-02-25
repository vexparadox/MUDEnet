#pragma once
#include <string>

class ClientState
{
private:
	const std::string m_username;
	const int m_id = -1;
	int m_locationid = 0;
public:
	ClientState(){};
	ClientState(int id, const std::string& username) : m_id(id), m_username(username) {};

	const std::string& Username() const {return m_username;}
	int ID() const { return m_id; }
	int LocationID() const {return m_locationid;}
};