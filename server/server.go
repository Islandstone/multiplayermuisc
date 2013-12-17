package main

import (
	"net"
	"os"
	"io"
	"container/list"
)

type Client struct {
	conn net.Conn

	send chan([]byte)
	broadcast chan([]byte)
	quit chan(bool)
}

type Manager struct {
	clients *list.List
}

var (
	mgr Manager = Manager{}
)

func main() {
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
			go NewClient(conn, broadcast)
		}
	}
}

func NewClient(conn net.Conn, broadcast chan([]byte)) {
	println("New client connected")

	c := &Client{conn, make(chan([]byte)), broadcast, make(chan(bool))}
	mgr.clients.PushBack(c)
	go ClientRecv(c)
	go ClientSend(c)
}

func Broadcaster(c chan []byte) {
	for {
		select {
		case msg := <-c:
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

func ReadBytes(conn net.Conn, buffer []byte) (err error) {
	var bytesRead = 0
	var n = 0
	for bytesRead < len(buffer) {
		n, err = conn.Read(buffer[bytesRead:])

		if err != nil {
			return
		}

		bytesRead += n
	}
	return nil
}

func ClientRecv(client *Client) {
	buffer := make([]byte, 32)

	for {
		err := ReadBytes(client.conn, buffer[0:2])

		if err != nil {
			if err == io.EOF {
				client.quit <- true
			}
			break
		}

		msg_type := buffer[1]

		if msg_type == MSG_PLAY { // Play
			println("Received play message from client")
			err := ReadBytes(client.conn, buffer[2:3])

			if err != nil {
				if err == io.EOF {
					client.quit <- true
				}
				break
			}

			println("Song id is", buffer[2])

			sendbuf := make([]byte, 3)
			copy(sendbuf, buffer)
			client.broadcast <- sendbuf
		} else if msg_type == MSG_STOP { // Stop
			println("Message type: Stop")

			client.broadcast <- buffer[0:2]
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
