import socket
from time import sleep

HOST = "127.0.0.1"
PORT = 8080

# Sends '1' to the client socket after n iterations of sending data
SET_KILL_SWITCH_AFTER_ITERATIONS = 6

def main():
  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    while True:
      iterations = 0
      print(f"Listening for new connection on {HOST}:{PORT}.")
      s.listen()
      conn, addr = s.accept()
      with conn:
        print(f"Connected by {addr}")
        while True:
          data = b'1' if iterations >= SET_KILL_SWITCH_AFTER_ITERATIONS else b'0'
          try:
            conn.send(data)
            sleep(1)
            iterations += 1
          except:
            print("Connection aborted by client, listening for new connection.")
            break


if __name__ == '__main__':
  main()