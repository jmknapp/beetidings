<?php
// Establish MySQL database connection
$servername = "localhost";
$username = "root";
$password = "R_250108_z";
$dbname = "air";

$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Process the selected time interval
$interval = $_POST['interval'];

switch ($interval) {
    case "hourly":
        $interval_sql = "DATE_FORMAT(`timestamp`, '%Y-%m-%d %H:00:00')";
        break;
    case "daily":
        $interval_sql = "DATE(`timestamp`)";
        break;
    case "weekly":
        $interval_sql = "CONCAT(YEARWEEK(`timestamp`, 3), '-01')";
        break;
    case "monthly":
        $interval_sql = "DATE_FORMAT(`timestamp`, '%Y-%m-01')";
        break;
    default:
        $interval_sql = "DATE_FORMAT(`timestamp`, '%Y-%m-%d %H:00:00')";
}

// Fetch data from the database
$sql = "SELECT $interval_sql as `interval`, AVG(`pm2p5`) as `avg_value` FROM `sen55` GROUP BY `interval`";
$result = $conn->query($sql);

$data = array();
while ($row = $result->fetch_assoc()) {
    $data[] = $row;
}

// Close the connection
$conn->close();

// Return the data as JSON
echo json_encode($data);
?>
