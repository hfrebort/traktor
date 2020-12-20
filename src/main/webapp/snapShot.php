<?php 

$data = file_get_contents('php://input');

if (isset($data)) {
	$data = json_decode($data, true);
	if (isset($data['url'])) {
		$command = sprintf("sudo -u pi python3 snapShot.py %s %d %d %d %d %d 2>&1", $data['url'], $data['cannyLowThreshold'], $data['cannyHighThreshold'], $data['houghThreshold'], $data['houghMinLineLength'], $data['houghMaxLineGap']);	
		system($command, $result);
		print_r($result); 
	} else {
		print(' !no url defined! '); 
	}
} else {
	print(' !no request body! '); 
}
?>