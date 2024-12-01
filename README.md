# warmars
2024 창업 경진대회 코드


---


### Architecture

rasberpi board (ubuntu  :   rasbery_pi_server.c <-> data.csv <-> app.py <-> index.html)

| ( serial communication )

arduino uno board 1,2 (wormars_adino.ino Upload, excute in board)


---


### rasbery pi board

#### rasbery_pi_server.c
- This C program communicates with the Arduino devices via serial connections and logs sensor data to a CSV file.

#### app.py
- This Python script implements the backend server using Flask and Flask-SocketIO. It also monitors a CSV file for changes, reflecting the latest data in the frontend.

#### index.html & app.js
- This HTML file provides the structure for displaying sensor data and the charts for visualization.
- This JavaScript file handles the frontend logic for displaying sensor data in real-time. It uses Chart.js for data visualization and Socket.IO for WebSocket communication.

---


### arduino uno board

#### wormars_adino.ino
- Arduino sketch collects sensor data and sends it to the Raspberry Pi via serial communication.
