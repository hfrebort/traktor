<?php 

$data = file_get_contents('php://input');

if (isset($data)) {
	$data = json_decode($data, true);
	if (isset($data['direction'])) {
		$command = sprintf("sudo python3 tractor.py %d %.2f %.2f %.2f %s 2>&1", $data['duration'], $data['left'], $data['center'], $data['right'], $data['direction']);	
		exec($command, $result);
		print_r($result); 
	} else {
		print(' !no direction! '); 
	}
} else {
	print(' !no request body! '); 
}
?>