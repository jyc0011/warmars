document.addEventListener("DOMContentLoaded", () => {
    var maxDataPoints = 10;
    var socket = io.connect(window.location.origin);

    function initializeChart(ctx, label) {
        return new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: label,
                    data: [],
                    borderColor: 'rgba(75, 192, 192, 1)',
                    borderWidth: 1,
                    fill: false
                }]
            },
            options: {
                responsive: true,
                scales: {
                    x: { display: true },
                    y: { display: true }
                }
            }
        });
    }

        // Arduino 1 차트 초기화
        var arduino1Charts = {
            ph: initializeChart(document.getElementById('arduino1-ph-chart').getContext('2d'), 'PH'),
            light: initializeChart(document.getElementById('arduino1-light-chart').getContext('2d'), 'Light'),
            soil: initializeChart(document.getElementById('arduino1-soil-chart').getContext('2d'), 'Soil Moisture'),
            humidity: initializeChart(document.getElementById('arduino1-humidity-chart').getContext('2d'), 'Humidity'),
            temperature: initializeChart(document.getElementById('arduino1-temperature-chart').getContext('2d'), 'Temperature')
        };

    socket.on("connect", () => {
        console.log("WebSocket 연결 성공");
    });

    socket.on("update_data", (data) => {
        console.log("수신된 데이터:", data);

        // Arduino 1 업데이트
        if (data.arduino_1) {
            document.getElementById('arduino1-ph').textContent = data.arduino_1.ph;
            document.getElementById('arduino1-light').textContent = data.arduino_1.light;
            document.getElementById('arduino1-soil').textContent = data.arduino_1.soil;
            document.getElementById('arduino1-humidity').textContent = data.arduino_1.humidity;
            document.getElementById('arduino1-temperature').textContent = data.arduino_1.temperature;

            // 차트 업데이트
            Object.keys(arduino1Charts).forEach(key => {
                let chart = arduino1Charts[key];
                chart.data.labels.push(new Date().toLocaleTimeString());
                chart.data.datasets[0].data.push(data.arduino_1[key]);
                if (chart.data.labels.length > maxDataPoints) {
                    chart.data.labels.shift();
                    chart.data.datasets[0].data.shift();
                }
                chart.update();
            });
        }

    });
});