import socket
import struct

from project_types import *
from network_codes import *
from word_entry import *

class client:
    def __init__(self, ip_address, port):
        self.ip_address = ip_address
        self.port = port


    def do_search(self, word_set, out_word_entries, out_file_table):
        try:
            self.connect_to_server()

            # Send command
            client_command = command.SEARCH
            if self.send_integer_value_and_handle(self.sock, client_command, code_type):
                return True, response.ERROR_RECEIVING_COMMAND

            # Send client data
            files_only = False
            if self.send_integer_value_and_handle(self.sock, files_only, 'B'):
                return True, response.ERROR_RECEIVING_DATA

            amount_of_words = len(word_set)
            if self.send_integer_value_and_handle(self.sock, amount_of_words, '>H'):
                return True, response.ERROR_RECEIVING_DATA

            for word in word_set:
                if self.send_size_and_string_and_handle(self.sock, word):
                    return True, response.ERROR_RECEIVING_DATA

            # Receive results
            _, response_code = self.recv_response_code(self.sock)
            if response_code != response.OK:
                self.close_connection(self.sock)
                return False, response_code

            found_files_amount = self.recv_integer_value(self.sock, id_type)
            for _ in range(found_files_amount):
                file_id = self.recv_integer_value(self.sock, id_type)
                _, filepath = self.recv_size_and_string_and_handle(self.sock)
                out_file_table[file_id] = filepath

            entries_amount = self.recv_integer_value(self.sock, '>Q')
            for _ in range(entries_amount):
                file_id = self.recv_integer_value(self.sock, id_type)
                position = self.recv_integer_value(self.sock, id_type)
                out_word_entries.update({file_id, position})

            self.close_connection(self.sock)
            return False, response_code

        except ConnectionError:
            self.close_connection(self.sock)
            return True, response.ERROR_RECEIVING_COMMAND


    def do_search_files_only(self, word_set, out_file_table):
        try:
            self.connect_to_server()

            # Send command
            client_command = command.SEARCH
            if self.send_integer_value_and_handle(self.sock, client_command, code_type):
                return True, response.ERROR_RECEIVING_COMMAND

            # Send client data
            files_only = True
            if self.send_integer_value_and_handle(self.sock, files_only, 'B'):
                return True, response.ERROR_RECEIVING_DATA

            amount_of_words = len(word_set)
            if self.send_integer_value_and_handle(self.sock, amount_of_words, '>H'):
                return True, response.ERROR_RECEIVING_DATA

            for word in word_set:
                if self.send_size_and_string_and_handle(self.sock, word):
                    return True, response.ERROR_RECEIVING_DATA

            # Receive results
            closed, response_code = self.recv_response_code(self.sock)
            if response_code != response.OK:
                self.close_connection()
                return False, response_code

            found_files_amount = self.recv_integer_value(self.sock, id_type)
            for _ in range(found_files_amount):
                _, filepath = self.recv_size_and_string_and_handle(self.sock)
                out_file_table.append(filepath)

            self.close_connection()
            return False, response_code

        except ConnectionError:
            self.close_connection()
            return True, response.ERROR_RECEIVING_COMMAND


    def connect_to_server(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.ip_address, self.port))

    def close_connection(self):
        self.sock.close()


    def recv_integer_value(self, sock, fmt):
        size = struct.calcsize(fmt)
        value_buffer = sock.recv(size)
    
        if len(value_buffer) != size:
            raise ConnectionError(f"Failed to receive the entire value.")
    
        return struct.unpack(fmt, value_buffer)[0]
    
    def recv_integer_value_and_handle(self, sock, fmt):
        try:
            value = self.recv_integer_value(sock, fmt)
            return False, value
        except (ConnectionError, struct.error):
            self.close_connection()
            return True, None
        
    def recv_response_code(self, sock):
        try:
            fmt = code_type
            recv_size = struct.calcsize(fmt)
            response_code = sock.recv(recv_size)

            if len(response_code) != recv_size:
                raise ConnectionError("Failed to receive the responce code.")

            response_code_value = struct.unpack(fmt, response_code)[0]

            return False, response_code_value

        except (ConnectionError, ValueError):
            self.close_connection()
            return True, None
        
    def send_integer_value(self, sock, value, fmt):
        value_buffer = struct.pack(fmt, value)
        sent = sock.send(value_buffer)
        
        if sent != len(value_buffer):
            raise ConnectionError("Failed to send the entire value.")
        
        return sent
    
    def send_integer_value_and_handle(self, sock, value, fmt):
        try:
            send_size = self.send_integer_value(sock, value, fmt)
            if send_size <= 0:
                self.close_connection()
                return True
            return False
        except ConnectionError:
            self.close_connection()
            return True

    def recv_size_and_string_and_handle(self, sock):
        try:
            byte_size_string = self.recv_integer_value(sock, '>H')
            
            total_received = 0
            received_data = bytearray()

            while total_received < byte_size_string:
                bytes_to_receive = min(1024, byte_size_string - total_received)
                chunk = sock.recv(bytes_to_receive)
                
                if not chunk:
                    self.close_connection()
                    return True, ""
                
                received_data.extend(chunk)
                total_received += len(chunk)

            out_string = received_data.decode('utf-8')
            return False, out_string

        except ConnectionError:
            self.close_connection()
            return True, ""

    def send_size_and_string_and_handle(self, sock, string):
        try:
            byte_size_string = len(string.encode('utf-8'))

            if self.send_integer_value_and_handle(sock, byte_size_string, '>H'):
                return True

            total_sent = 0
            encoded_string = string.encode('utf-8')

            while total_sent < byte_size_string:
                bytes_to_send = min(1024, byte_size_string - total_sent)
                chunk = encoded_string[total_sent : total_sent + bytes_to_send]

                sent_size = sock.send(chunk)
                if sent_size <= 0:
                    self.close_connection()
                    return True

                total_sent += sent_size

            return False

        except ConnectionError:
            self.close_connection()
            return True
