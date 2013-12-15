package main

import (
	"net"
	"os"
	"time"
	"io"
	"strings"
	"sync"
	"container/list"
)

type Message struct {
	msg []byte
}

type Client struct {
	conn net.Conn

	send chan([]byte)
	broadcast chan([]byte)
	quit chan(bool)

	sendbuf []Message
}

type Manager struct {
	lock sync.RWMutex // TODO
	//clients []*Client
	clients *list.List
}

const (
	RECV_BUF_LEN = 128
)

var (
	mgr Manager = Manager{}
)


func (m *Manager) broadcast(msg []byte) {
	for i := 0; i < m.clients.Len(); i++ {
		println("Broadcasting " + string(msg))
		//mgr.clients[i].sendbuf = append(mgr.clients[i].sendbuf, Message{msg})
	}
}

func main() {
	//mgr.clients = make([]*Client,0,32)
	mgr.clients = list.New()
	broadcast := make(chan([]byte))
	go Broadcaster(broadcast)

	listener, err := net.Listen("tcp", "0.0.0.0:9090")

	if err != nil {
		println(err)
		os.Exit(1)
	}

	println("Server is now running")

	for {
		conn, err := listener.Accept()

		if err != nil {
			println(err)
			continue
		} else {
			//go serverFunc(conn)
			go NewClient(conn, broadcast)
		}
	}
}

// Returns true if error was handled.
// Should only handle fatal errors
func errorHandle(conn net.Conn, err error) bool {
	switch err {
	case io.EOF:
		println("Closed connection")
		conn.Close()
		conn = nil
		return true
	}
	return false
}

func serverFunc(conn net.Conn) {
	buf := make([]byte, RECV_BUF_LEN)

	/*
	conn.SetReadDeadline(time.Now().Add(30 * time.Second) )

	n, err := conn.Read(buf)
	if err != nil {
		if neterr, ok := err.(net.Error); ok && neterr.Timeout() {
			println("Client did not send VED in time, bye.")
			conn.Close()
			conn = nil
		} else {
			errorHandle(conn, err)
		}

		return
	}

	if strings.TrimSpace(string(buf)[0:3]) != "VED" {
		println("Malformed connection request, bye.")
		conn.Close()
		return
	}

	conn.Write([]byte("VCA\n"))
	*/

	client := Client{}
	client.conn = conn
	client.sendbuf = make([]Message, 0, 10)

	//mgr.clients = append(mgr.clients, &client)

	for i := range client.sendbuf {
		client.sendbuf[i].msg = make([]byte,0,64)
	}


	for {
		buf = make([]byte, RECV_BUF_LEN)

		conn.SetReadDeadline(time.Now().Add(100 * time.Millisecond) )

		n, err := conn.Read(buf)
		if err != nil {
			if neterr, ok := err.(net.Error); ok && neterr.Timeout() {
				//println("Timeout")
			} else {
				if errorHandle(conn, err) {
					return
				}
			}
		} else {
			println("Received \"" + strings.TrimSpace(string(buf[0:n])) + "\" from client")
			mgr.broadcast(buf[0:n])
		}

		/*
		if len(mgr.clients[0].sendbuf) > 0 {
			conn.SetWriteDeadline(time.Now().Add(100 * time.Millisecond) )

			// TODO: Incomplete send
			println("Sending " + string(client.sendbuf[0].msg) )
			conn.Write(client.sendbuf[0].msg)

			client.sendbuf = client.sendbuf[1:]
		}
		*/

		/*
		_, err = conn.Write(buf)
		if err != nil {
			if neterr, ok := err.(net.Error); ok && neterr.Timeout() {
				println("Write timeout")
			} else {
				if errorHandle(conn, err) {
					return
				}
			}
		}
		*/
	}
}

func NewClient(conn net.Conn, broadcast chan([]byte)) {
	println("New client connected")

	c := &Client{conn, make(chan([]byte)), broadcast, make(chan(bool)), make([]Message, 0)}
	//mgr.clients = append(mgr.clients, c)
	mgr.clients.PushBack(c)
	go ClientRecv(c)
	go ClientSend(c)
}

func Broadcaster(c chan []byte) {
	for {
		select {
		case msg := <-c:
			println("Broadcasting")
			for e := mgr.clients.Front(); e != nil; e = e.Next() {
				client := e.Value.(*Client)
				client.send <- msg
			}
		}
	}
}

const (
	MSG_STOP uint8 = 0
	MSG_PLAY uint8 = 1
)

func ClientRecv(client *Client) {
	for {
		header := make([]byte, 2)
		_, err := client.conn.Read(header)

		if err != nil {
			if err == io.EOF {
				client.quit <- true
			}
			break
		}

		msg_type := header[1]

		println("Received message from client")
		println(header[0])
		println(header[1])

		if msg_type == MSG_PLAY { // Play
			println("Message type: Play")
			msg_buf := make([]byte, 1)
			_, err := client.conn.Read(msg_buf)

			if err != nil {
				println("Read error in play msg")
			}

			song_id := msg_buf[0]
			println("Song id is", song_id)

			broadcast_msg := make([]byte, 2)
			broadcast_msg[0] = MSG_PLAY
			broadcast_msg[1] = song_id

			client.broadcast <- broadcast_msg
		} else if msg_type == MSG_STOP { // Stop
			println("Message type: Stop")
			broadcast_msg := make([]byte, 1)
			broadcast_msg[0] = MSG_STOP
			client.broadcast <- broadcast_msg
		}
	}
}

func ClientSend(client *Client) {
	for {
		select {
		case msg := <-client.send:
			var n int
			var err error
			for written := 0; written < len(msg); written += n {
				n, err = client.conn.Write(msg[written:])

				if err != nil {
					println("Send err: ", err.Error())
					client.quit <- true;
					break
				}
			}
		case <-client.quit:
			println("Client quit")
			client.conn.Close()

			for e := mgr.clients.Front(); e != nil; e = e.Next() {
				other := e.Value.(*Client)

				if other.conn == client.conn {
					mgr.clients.Remove(e)
					break
				}
			}

			break
		}
	}
}
