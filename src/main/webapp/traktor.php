<!doctype html>
<html data-ng-app="tractor">
<head>
<!-- Required meta tags -->
<meta charset="utf-8">
<meta name="viewport"
	content="width=device-width, initial-scale=1, shrink-to-fit=no">

<!-- Bootstrap CSS -->
<!-- Latest compiled and minified CSS -->
<link rel="stylesheet" href="css/bootstrap.min.css">
<script type="text/javascript">
    //Selector for your <video> element
    const video = document.querySelector('#videoPlayer');

    //Core
    window.navigator.mediaDevices.getUserMedia({ video: true })
        .then(stream => {
            video.srcObject = stream;
            video.onloadedmetadata = (e) => {
                video.play();
            };
        })
        .catch( () => {
            alert('You have give browser the permission to run Webcam and mic ;( ');
        });

</script>
<title>Traktor</title>
</head>
<body>
<?php 
if (isset($_POST['direction'])) {
	$command = sprintf("sudo python3 controlTractor.py %d %.2f %.2f %.2f %s 2>&1", $_POST['duration'], $_POST['left'], $_POST['center'], $_POST['right'], $_POST['direction']);	
	exec($command, $result);
}
?>
	<div class="container-fluid">
		<div class="row">
			<div class="col-sm-6">
				<!-- 16:9 aspect ratio -->
				<div class="embed-responsive embed-responsive-16by9">
					<!-- <video id="videoPlayer" controls muted autoplay></video> -->
					<video src="soja.mp4" type="video/mp4" autobuffer autoplay controls />
				</div>			
				<p><?php echo $command; ?></p>
				<p><?php echo print_r($result); ?></p>
			</div>
		
			<div class="col-sm-6">
				<form name="tractorForm" method="post">
					<div class="row"><div class="col-sm-12">&nbsp;</div></div>
					<div class="row">
						<div class="col-sm-4">
							<button name="direction" value="left" class="btn btn-primary btn-block btn-lg">
								<span class="glyphicon glyphicon-chevron-left"></span>
							</button>
						</div>
						<div class="col-sm-4">
							<button name="direction" value="center" class="btn btn-primary btn-block btn-lg">
								<span class="glyphicon glyphicon-align-justify"></span>
							</button>
						</div>
						<div class="col-sm-4">
							<button name="direction" value="right" class="btn btn-primary btn-block btn-lg">
								<span class="glyphicon glyphicon-chevron-right"></span>
							</button>
						</div>
					</div>
					<div class="row"><div class="col-sm-12">&nbsp;</div></div>
					<div class="row">
						<div class="col-sm-12">
							<button name="direction" value="stop" class="btn btn-block btn-lg btn-danger">
								<span class="glyphicon glyphicon-off"></span>
							</button>
						</div>
					</div>
					<div class="row"><div class="col-sm-12">&nbsp;</div></div>
					<div class="row">
						<div class="col-sm-3"><input name="duration" class="form-control" value="4" /></div>
						<div class="col-sm-3"><input name="left" class="form-control" value="0.25" /></div>
						<div class="col-sm-3"><input name="center" class="form-control" value="0.5" /></div>
						<div class="col-sm-3"><input name="right" class="form-control" value="1" /></div>
					</div>
				</form>
			</div>
		</div>
	</div>
</body>
</html>