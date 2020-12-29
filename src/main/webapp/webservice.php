<?php 
$startTime = microtime(true);

system(sprintf("sudo -u pi ./webServiceOnOff.sh 2>&1"), $result);
print_r($result); 	

$time = microtime(true) - $startTime;
print(sprintf('Execution time: %f in ms', $time));
?>