import socket
import threading
import time
import random

# Configuration
HOST = '127.0.0.1'
PORT = 12345

class SmartHomeSimulator:
    def __init__(self):
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.bind((HOST, PORT))
        self.server_socket.listen(5)
        self.running = True
        
        # Device states
        self.devices = {
            "light_living": "OFF",
            "light_bedroom": "OFF",
            "light_kitchen": "OFF",
            "light_restroom": "OFF",
            "light_dining": "OFF",
            "fan_kitchen": "OFF",
            "ac_living": "OFF",
            "curtain_living": "0", # 0-100%
            "temp_sensor": 25.0,
            "humid_sensor": 60.0
        }
        
        print(f"[*] Smart Home Device Simulator started on {HOST}:{PORT}")

    def handle_client(self, client_socket):
        print(f"[*] Accepted connection from {client_socket.getpeername()}")
        
        # Start a thread to send sensor data periodically
        sensor_thread = threading.Thread(target=self.send_sensor_data, args=(client_socket,))
        sensor_thread.daemon = True
        sensor_thread.start()

        try:
            while self.running:
                data = client_socket.recv(1024)
                if not data:
                    break
                
                command = data.decode('utf-8').strip()
                print(f"[>] Received command: {command}")
                
                response = self.process_command(command)
                if response:
                    client_socket.send(response.encode('utf-8'))
                    print(f"[<] Sent response: {response}")
                    
        except ConnectionResetError:
            print("[!] Connection reset by client")
        finally:
            client_socket.close()
            print("[*] Connection closed")

    def process_command(self, cmd):
        # Format: DEVICE_ACTION_PARAM (e.g., LIGHT_LIVING_ON)
        parts = cmd.split('_')
        if len(parts) < 2:
            return "ERROR_INVALID_CMD"
        
        # Generic handling for simple ON/OFF devices
        # Try to map command to device key
        # e.g. LIGHT_LIVING_ON -> light_living = ON
        # e.g. FAN_KITCHEN_OFF -> fan_kitchen = OFF
        
        # Determine action (ON/OFF)
        action = parts[-1]
        if action in ["ON", "OFF"]:
            # Reconstruct device key from parts[0]...parts[-2]
            # e.g. LIGHT_LIVING -> light_living
            device_key = "_".join(parts[:-1]).lower()
            
            if device_key in self.devices:
                self.devices[device_key] = action
                return f"{cmd}_OK"
                
        # Specific handling for special devices
        if cmd.startswith("AC_TEMP_"):
            # AC_TEMP_26
            try:
                temp = parts[2]
                return f"AC_TEMP_{temp}_OK"
            except:
                return "ERROR_PARAM"
        elif "AC_" in cmd and "_TEMP_" in cmd:
            # AC_LIVING_TEMP_26
            try:
                temp = parts[3]
                return f"{cmd}_OK"
            except:
                return "ERROR_PARAM"
        elif "AC_" in cmd and "_MODE_" in cmd:
            # AC_LIVING_MODE_COOL
            return f"{cmd}_OK"
        elif "AC_" in cmd and "_FAN_" in cmd:
            # AC_LIVING_FAN_AUTO
            return f"{cmd}_OK"
        elif "LIGHT_" in cmd and "_BRI_" in cmd:
            # LIGHT_LIVING_BRI_80
            return f"{cmd}_OK"
        elif "LIGHT_" in cmd and "_COLOR_" in cmd:
            # LIGHT_LIVING_COLOR_WARM
            return f"{cmd}_OK"
        elif cmd == "CURTAIN_LIVING_TOGGLE":
            current_val = int(self.devices.get("curtain_living", "0"))
            # Toggle between 0 and 100
            new_val = "100" if current_val == 0 else "0"
            self.devices["curtain_living"] = new_val
            return f"CURTAIN_LIVING_{new_val}_OK"
        elif cmd == "GET_ALL_STATUS":
            # Return JSON-like string or simple formatted string
            status = []
            for k, v in self.devices.items():
                status.append(f"{k}:{v}")
            return ",".join(status)
            
        return "UNKNOWN_CMD"

    def send_sensor_data(self, client_socket):
        while self.running:
            try:
                # Simulate environmental changes
                self.devices["temp_sensor"] += random.uniform(-0.5, 0.5)
                self.devices["humid_sensor"] += random.uniform(-1, 1)
                
                # Keep within realistic bounds
                self.devices["temp_sensor"] = max(15, min(35, self.devices["temp_sensor"]))
                self.devices["humid_sensor"] = max(30, min(90, self.devices["humid_sensor"]))
                
                msg = f"ENV_DATA:TEMP={self.devices['temp_sensor']:.1f},HUMID={self.devices['humid_sensor']:.1f}"
                # client_socket.send(msg.encode('utf-8')) # Optional: Auto-push data
                time.sleep(5) 
            except:
                break

    def start(self):
        while self.running:
            client, addr = self.server_socket.accept()
            client_handler = threading.Thread(target=self.handle_client, args=(client,))
            client_handler.start()

if __name__ == "__main__":
    sim = SmartHomeSimulator()
    try:
        sim.start()
    except KeyboardInterrupt:
        print("\n[*] Stopping simulator...")
