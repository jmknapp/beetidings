<?php
$host = 'localhost';
$db   = 'air';
$user = 'airreader';
$pass = 'beebuzzbuzz';
$charset = 'utf8mb4';

date_default_timezone_set('America/New_York');

$dsn = "mysql:host=$host;dbname=$db;charset=$charset";
$options = [
    PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
];

try {
    $pdo = new PDO($dsn, $user, $pass, $options);

    if (isset($_GET['datesOnly'])) {
        $stmt = $pdo->query("SELECT DISTINCT DATE(timestamp) as day FROM sen55 ORDER BY day ASC");
        $dates = $stmt->fetchAll(PDO::FETCH_COLUMN);
        echo json_encode($dates);
        exit;
    }

    // Get start and end from URL if provided, otherwise use today's range
    $start = isset($_GET['start']) ? $_GET['start'] . ' 00:00:00' : date('Y-m-d 00:00:00');
    $end   = isset($_GET['end']) ? $_GET['end'] . ' 23:59:59' : date('Y-m-d 23:59:59');

    // get number of data points in date range
    $countStmt = $pdo->prepare("SELECT COUNT(*) FROM sen55 WHERE timestamp BETWEEN :start AND :end");
    $countStmt->execute(['start' => $start, 'end' => $end]);
    $rowCount = $countStmt->fetchColumn();

    error_log("Start: " . $start) ;
    error_log("End: " . $end) ;
    error_log("N points: " . $rowCount) ;

    if ($rowCount < 4000) {
      $stmt = $pdo->prepare("
          SELECT 
            DATE_FORMAT(timestamp, '%Y-%m-%dT%H:%i:%s') AS timestamp,
            ROUND(pm1p0,1) pm1p0, ROUND(pm2p5-pm1p0,1) pm2p5, ROUND(pm4p0-pm2p5,1) pm4p0, ROUND(pm10p0-pm4p0,1) pm10p0, humidity, temperature, voc, nox
          FROM sen55
          WHERE timestamp BETWEEN :start AND :end
          ORDER BY timestamp ASC
      ");
    } elseif ($rowCount < 240000) {
	$stmt = $pdo->prepare("
          SELECT
              DATE_FORMAT(timestamp, '%Y-%m-%dT%H:%i:00') AS timestamp,
              ROUND(MAX(pm1p0),1) AS pm1p0,
              ROUND(MAX(pm2p5-pm1p0),1) AS pm2p5,
              ROUND(MAX(pm4p0-pm2p5),1) AS pm4p0,
              ROUND(MAX(pm10p0-pm4p0),1) AS pm10p0,
              ROUND(AVG(humidity),1) AS humidity,
              ROUND(AVG(temperature),1) AS temperature,
              ROUND(MAX(voc),1) AS voc,
              ROUND(MAX(nox),1) AS nox
          FROM sen55
          WHERE timestamp BETWEEN :start AND :end
          GROUP BY DATE_FORMAT(timestamp, '%Y-%m-%d %H:%i')
          ORDER BY timestamp ASC
      ");
    } else {
	$stmt = $pdo->prepare("
          SELECT
              DATE_FORMAT(timestamp, '%Y-%m-%dT%H:%i:00') AS timestamp,
              ROUND(MAX(pm1p0),1) AS pm1p0,
              ROUND(MAX(pm2p5-pm1p0),1) AS pm2p5,
              ROUND(MAX(pm4p0-pm2p5),1) AS pm4p0,
              ROUND(MAX(pm10p0-pm4p0),1) AS pm10p0,
              ROUND(AVG(humidity),1) AS humidity,
              ROUND(AVG(temperature),1) AS temperature,
              ROUND(MAX(voc),1) AS voc,
              ROUND(MAX(nox),1) AS nox
          FROM sen55
          WHERE timestamp BETWEEN :start AND :end
          GROUP BY DATE_FORMAT(timestamp, '%Y-%m-%d %H')
          ORDER BY timestamp ASC
      ");
    }
    //$sqlDebug = $stmt->queryString;
    //$sqlDebug = str_replace(':start', "'$start'", $sqlDebug);
    //$sqlDebug = str_replace(':end', "'$end'", $sqlDebug);
    //error_log("Final SQL: " . $sqlDebug);

    $stmt->execute(['start' => $start, 'end' => $end]);
    $data = $stmt->fetchAll();

    header('Content-Type: application/json');
    echo json_encode($data);

} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['error' => $e->getMessage()]);
    exit;
}
?>
