// Cấu hình mảng lưu trữ dữ liệu cho biểu đồ đường (Line Charts)
const maxDataPoints = 300;
let timeLabels = [];
let chartDataTemp = [], chartDataHumi = [], chartDataPM25 = [];

// 1. Khởi tạo biểu đồ đường (Line Charts)
// Khởi tạo biểu đồ Nhiệt độ (Line Chart)
const ctxTemp = document.getElementById('chartTemp').getContext('2d');
const chartTemp = new Chart(ctxTemp, {
    type: 'line',
    data: {
        labels: timeLabels,
        datasets: [{ 
            label: 'Temp (°C)', 
            borderColor: '#ff6384', 
            data: chartDataTemp, 
            tension: 0.3,
            fill: true, 
            backgroundColor: 'rgba(255, 99, 132, 0.2)' 
        }]
    },
    options: { animation: false, scales: { x: { display: false } } }
});

// Khởi tạo biểu đồ Độ ẩm (Line Chart)
const ctxHumi = document.getElementById('chartHumi').getContext('2d');
const chartHumi = new Chart(ctxHumi, {
    type: 'line',
    data: {
        labels: timeLabels,
        datasets: [{ 
            label: 'Humi (%)', 
            borderColor: '#36a2eb', 
            data: chartDataHumi, 
            tension: 0.3,
            fill: true, 
            backgroundColor: 'rgba(54, 162, 235, 0.2)' 
        }]
    },
    options: { animation: false, scales: { x: { display: false } } }
});

// (Biểu đồ PM2.5 chartPM giữ nguyên không cần sửa)

const ctxPM = document.getElementById('chartPM').getContext('2d');
const chartPM = new Chart(ctxPM, {
    type: 'line',
    data: {
        labels: timeLabels,
        datasets: [{ label: 'PM2.5 (µg/m³)', borderColor: '#cc65fe', data: chartDataPM25, tension: 0.3, fill: true, backgroundColor: 'rgba(204, 101, 254, 0.2)' }]
    },
    options: { animation: false, scales: { x: { display: false } } }
});

// 2. Hàm cấu hình biểu đồ Gauge (Dạng nửa hình tròn)
function createGauge(ctxId, color, maxVal) {
    return new Chart(document.getElementById(ctxId), {
        type: 'doughnut',
        data: { datasets: [{ data: [0, maxVal], backgroundColor: [color, '#333'], borderWidth: 0 }] },
        options: { rotation: -90, circumference: 180, cutout: '80%', animation: false, tooltips: { enabled: false } }
    });
}

const gaugeTemp = createGauge('gaugeTemp', '#ff6384', 50);
const gaugeHumi = createGauge('gaugeHumi', '#36a2eb', 100);
const gaugePM25 = createGauge('gaugePM25', '#cc65fe', 150);
const gaugeCO2 = createGauge('gaugeCO2', '#ffce56', 5000);

// 3. Hàm Fetch API lấy dữ liệu từ ESP32 mỗi 2 giây
async function fetchSensorData() {
    try {
        const response = await fetch('/api/data');
        const data = await response.json();

        // Cập nhật Text
        document.getElementById('val-temp').innerText = data.temperature.toFixed(1);
        document.getElementById('val-humi').innerText = data.humidity.toFixed(1);
        document.getElementById('val-pres').innerText = data.pressure.toFixed(1);
        document.getElementById('val-gas').innerText = data.gas_res.toFixed(2);
        document.getElementById('val-pm1').innerText = data.pm1;
        document.getElementById('val-pm25').innerText = data.pm25;
        document.getElementById('val-pm10').innerText = data.pm10;
        document.getElementById('val-co2').innerText = data.co2;
        document.getElementById('rtc-time').innerText = data.time;
        document.getElementById('sd-status').innerText = data.sd_ok ? "Hoạt động" : "Lỗi";

        // Cập nhật Trạng thái AQI
        const aqiEl = document.getElementById('aqi-status');
        aqiEl.innerText = "CHẤT LƯỢNG: " + data.aqi_status;
        aqiEl.className = "p-3 rounded fw-bold text-white ";
        if (data.aqi_status === "GOOD") aqiEl.classList.add("aqi-good");
        else if (data.aqi_status === "MODERATE") aqiEl.classList.add("aqi-moderate");
        else aqiEl.classList.add("aqi-unhealthy");

        // Cập nhật Gauges
        gaugeTemp.data.datasets[0].data = [data.temperature, 50 - data.temperature]; gaugeTemp.update();
        gaugeHumi.data.datasets[0].data = [data.humidity, 100 - data.humidity]; gaugeHumi.update();
        gaugePM25.data.datasets[0].data = [data.pm25, 150 - data.pm25]; gaugePM25.update();
        gaugeCO2.data.datasets[0].data = [data.co2, 5000 - data.co2]; gaugeCO2.update();

        // Cập nhật Line Charts
        if (timeLabels.length > maxDataPoints) {
            timeLabels.shift(); chartDataTemp.shift(); chartDataHumi.shift(); chartDataPM25.shift();
        }
        timeLabels.push(data.time);
        chartDataTemp.push(data.temperature);
        chartDataHumi.push(data.humidity);
        chartDataPM25.push(data.pm25);

        chartTemp.update();
        chartHumi.update();
        chartPM.update();

    } catch (error) {
        console.error("Lỗi mất kết nối ESP32:", error);
    }
}

// Chạy vòng lặp lấy mẫu
setInterval(fetchSensorData, 2000);
fetchSensorData();