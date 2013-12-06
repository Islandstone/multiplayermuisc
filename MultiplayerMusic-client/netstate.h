#ifndef NETSTATE_H
#define NETSTATE_H

class NetState
{
public:
	NetState* get() {
		static NetState ns;
		return &ns;
	}

	void send_msg();
	void recv_msg();

private:
	NetState();

};

#endif // NETSTATE_H
