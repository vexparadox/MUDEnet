#pragma once
#include <stdio.h>
#include <cstring>
#include <iostream>

using Byte = unsigned char;

class DataStream
{
	const Byte* m_data = nullptr;
	const size_t m_data_size = 0;
	const Byte* m_head = nullptr;
	bool m_self_allocation = false;
public:
	DataStream(size_t data_size) : m_data_size(data_size)
	{
		m_data = (Byte*)malloc(sizeof(Byte)*m_data_size);
		memset((void*)m_data, 0, m_data_size);
		m_head = m_data;
		m_self_allocation = true;
	};

	DataStream(const Byte* data, size_t data_size) : m_data(data), m_head(data), m_data_size(data_size){};

	~DataStream() 
	{
		if(m_self_allocation)
		{
			delete m_data;
			m_data = nullptr;
		}
	}

	const Byte* data() const { return m_data; }

	const Byte* head() const { return m_head; }

	size_t size() { return m_data_size; }

	void dump()
	{
		for(int i = 0; i < m_data_size; i++)
		{
			printf("%c", m_data[i]);
		}
	}

	void reset_head()
	{
		m_head = m_data;
	}

	void clear_data()
	{
		reset_head();
		memset((void*)m_data, 0, m_data_size);
	}

	bool skip_forwards(size_t jump_size)
	{
		if(m_head+jump_size <= m_data+m_data_size)
		{
			m_head += jump_size;
			return true;
		}
		std::cout << "Attempt to skip past the end of a data stream" << std::endl;
		return false;
	}

	bool jump_to(size_t location)
	{
		if(m_head+location <= m_data+m_data_size)
		{
			m_head = m_data+location;
			return true;
		}
		std::cout << "Attempt to jump past the end of a data stream" << std::endl;
		return false;
	}

	std::string string(size_t size)
	{
		std::string str;
		if(m_head+size <= m_data+m_data_size)
		{
			str = std::string((char*)(m_head), size);
			m_head += size;
			return str;
		}
		std::cout << "Attempt to read invalid memory size to data stream" << std::endl;
		return str;
	}

	template <typename T>
	bool read(T& t)
	{
		if(m_head+sizeof(t) <= m_data+m_data_size)
		{
			t = *(T*)(m_head);
			m_head += sizeof(t);
			return true;
		}
		std::cout << "Attempt to read invalid memory size to data stream" << std::endl;
		return false;
	}

	template <typename T>
	T* read()
	{
		if(m_head+sizeof(T) <= m_data+m_data_size)
		{
			T* t = (T*)m_head;
			m_head += sizeof(t);
			return t;
		}
		std::cout << "Attempt to read invalid memory size to data stream" << std::endl;
		return nullptr;
	}

	bool write(const std::string& str)
	{
		return write(str.c_str(), str.size());
	}

	template <typename T>
	bool write(const T* t, size_t size = 0)
	{
		if(size == 0)
		{
			size = sizeof(*t);
		}
		if(m_head+size <= m_data+m_data_size)
		{
			memcpy((char*)m_head, t, size);
			m_head += size;
			return true;
		}
		std::cout << "Attempt to write invalid memory size to data stream" << std::endl;
		return false;
	}

	template <typename T>
	bool write(const T& t)
	{
		if(m_head+sizeof(t) <= m_data+m_data_size)
		{
			*(T*)m_head = t;
			m_head += sizeof(t);
			return true;
		}
		std::cout << "Attempt to write invalid memory size to data stream" << std::endl;
		return false;
	}

	template <typename T>
	bool write_at(size_t position, const T& t)
	{
		if(m_data+position+sizeof(t) <= m_data+m_data_size)
		{
			*(T*)(m_data+position) = t;
			return true;
		}
		std::cout << "Attempt to write invalid memory size to data stream" << std::endl;
		return false;
	}

	template <typename T>
	bool write_at(size_t position, const T* t, size_t size = 0)
	{
		if(size == 0)
		{
			size = sizeof(*t);
		}
		if(m_head+position+size <= m_data+m_data_size)
		{
			memcpy((char*)m_data+position, t, size);
			m_head += size;
			return true;
		}
		std::cout << "Attempt to write invalid memory size to data stream" << std::endl;
		return false;
	}

};
