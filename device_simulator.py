import socket
import threading
import time
import random
import json
from datetime import datetime, timedelta

# Configuration
HOST = '127.0.0.1'
PORT = 12345

class SmartHomeSimulator:
    def __init__(self):
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.bind((HOST, PORT))
        self.server_socket.listen(5)
        self.running = True
        
        # Device states (dynamic, will be created on first command)
        self.devices = {}
        
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
        
        # Determine action (ON/OFF)
        action = parts[-1]
        if action in ["ON", "OFF"]:
            # Reconstruct device key from parts[0]...parts[-2]
            # e.g. LIGHT_LIVING -> light_living
            device_key = "_".join(parts[:-1]).lower()
            
            # Dynamic device creation: if device doesn't exist, create it
            if device_key not in self.devices:
                self.devices[device_key] = action
                print(f"[+] Dynamic device created: {device_key}")
                return f"{cmd}_OK"
            
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
        elif cmd.startswith("FAN_") and action in ["ON", "OFF"]:
            # FAN_KITCHEN_ON
            device_key = "_".join(parts[:-1]).lower()
            if device_key not in self.devices:
                self.devices[device_key] = action
                print(f"[+] Dynamic device created: {device_key}")
            return f"{cmd}_OK"
        elif "AC_" in cmd and "_TEMP_" in cmd:
            # AC_LIVING_TEMP_26
            try:
                temp = parts[3]
                device_key = "_".join(parts[1:3]).lower()
                # Dynamic device creation for AC
                if device_key not in self.devices:
                    self.devices[device_key] = "OFF"
                    print(f"[+] Dynamic device created: {device_key}")
                return f"{cmd}_OK"
            except:
                return "ERROR_PARAM"
        elif "AC_" in cmd and "_MODE_" in cmd:
            # AC_LIVING_MODE_COOL
            try:
                device_key = "_".join(parts[1:3]).lower()
                if device_key not in self.devices:
                    self.devices[device_key] = "OFF"
                    print(f"[+] Dynamic device created: {device_key}")
                return f"{cmd}_OK"
            except:
                return "ERROR_PARAM"
        elif "AC_" in cmd and "_FAN_" in cmd:
            # AC_LIVING_FAN_AUTO
            try:
                device_key = "_".join(parts[1:3]).lower()
                if device_key not in self.devices:
                    self.devices[device_key] = "OFF"
                    print(f"[+] Dynamic device created: {device_key}")
                return f"{cmd}_OK"
            except:
                return "ERROR_PARAM"
        elif "LIGHT_" in cmd and "_BRI_" in cmd:
            # LIGHT_LIVING_BRI_80
            try:
                device_key = "_".join(parts[1:3]).lower()
                if device_key not in self.devices:
                    self.devices[device_key] = "OFF"
                    print(f"[+] Dynamic device created: {device_key}")
                return f"{cmd}_OK"
            except:
                return "ERROR_PARAM"
        elif "LIGHT_" in cmd and "_COLOR_" in cmd:
            # LIGHT_LIVING_COLOR_WARM
            try:
                device_key = "_".join(parts[1:3]).lower()
                if device_key not in self.devices:
                    self.devices[device_key] = "OFF"
                    print(f"[+] Dynamic device created: {device_key}")
                return f"{cmd}_OK"
            except:
                return "ERROR_PARAM"
        elif cmd.startswith("CURTAIN_") and len(parts) >= 3:
            # Handle curtain position commands: CURTAIN_LIVING_100, CURTAIN_BEDROOM_50
            try:
                device_key = "_".join(parts[:-1]).lower()
                position = parts[-1]
                if position.isdigit():
                    pos_val = int(position)
                    if 0 <= pos_val <= 100:
                        if device_key not in self.devices:
                            self.devices[device_key] = "0"
                            print(f"[+] Dynamic device created: {device_key}")
                        self.devices[device_key] = position
                        return f"{cmd}_OK"
            except:
                pass
        elif cmd == "CURTAIN_LIVING_TOGGLE":
            current_val = int(self.devices.get("curtain_living", "0"))
            # Toggle between 0 and 100
            new_val = "100" if current_val == 0 else "0"
            self.devices["curtain_living"] = new_val
            return f"CURTAIN_LIVING_{new_val}_OK"
        elif cmd.startswith("CURTAIN_") and cmd.endswith("_TOGGLE"):
            # Generic curtain toggle for any curtain device
            # e.g., CURTAIN_BEDROOM_TOGGLE
            parts = cmd.split('_')
            if len(parts) >= 2:
                curtain_id = "_".join(parts[1:-1]).lower()
                if curtain_id not in self.devices:
                    self.devices[curtain_id] = "0"
                    print(f"[+] Dynamic device created: {curtain_id}")
                current_val = int(self.devices.get(curtain_id, "0"))
                new_val = "100" if current_val == 0 else "0"
                self.devices[curtain_id] = new_val
                return f"{cmd}_{new_val}_OK"
        elif cmd == "GET_ALL_STATUS":
            # Return JSON-like string or simple formatted string
            status = []
            for k, v in self.devices.items():
                status.append(f"{k}:{v}")
            return ",".join(status)
        elif cmd == "GET_HISTORY_ENV_DATA":
            # Simulate last 7 days of data, one point per hour
            history_data = []
            now = datetime.now()
            # Start from 7 days ago
            start_time = now - timedelta(days=7)
            
            current_time = start_time
            while current_time <= now:
                # Simulate some daily cycle for temp
                hour = current_time.hour
                base_temp = 20 + 5 * (1 - abs(hour - 14) / 12) # Peak at 14:00
                temp = base_temp + random.uniform(-1, 1)
                humid = 50 + random.uniform(-10, 10)
                
                history_data.append({
                    "timestamp": current_time.strftime("%Y-%m-%d %H:%M:%S"),
                    "temperature": round(temp, 1),
                    "humidity": round(humid, 1)
                })
                current_time += timedelta(hours=4) # Every 4 hours to keep payload reasonable
            
            json_str = json.dumps(history_data)
            return f"HISTORY_ENV_DATA:{json_str}"
            
        return "UNKNOWN_CMD"

    def send_sensor_data(self, client_socket):
        while self.running:
            try:
                # Initialize sensor devices if not exists
                if "temp_sensor" not in self.devices:
                    self.devices["temp_sensor"] = 25.0
                if "humid_sensor" not in self.devices:
                    self.devices["humid_sensor"] = 60.0
                
                # Simulate environmental changes
                self.devices["temp_sensor"] += random.uniform(-2.0, 2.0)
                self.devices["humid_sensor"] += random.uniform(-5.0, 5.0)
                
                # Keep within realistic bounds
                self.devices["temp_sensor"] = max(15, min(35, self.devices["temp_sensor"]))
                self.devices["humid_sensor"] = max(30, min(90, self.devices["humid_sensor"]))
                
                msg = f"ENV_DATA:TEMP={self.devices['temp_sensor']:.1f},HUMID={self.devices['humid_sensor']:.1f}"
                try:
                    client_socket.send(msg.encode('utf-8'))
                except OSError:
                    break
                time.sleep(5) 
            except Exception as e:
                print(f"Error sending sensor data: {e}")
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
