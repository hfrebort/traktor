angular.module('tractor',[])
.controller('TractorController', function($scope, $http) {
	// initialize the content type of post request with text/plaien
	// TO AVOID triggering the pre-flight OPTIONS request
	$http.defaults.headers.post["Content-Type"] = "text/plain";
	
	const host = 'http://10.3.141.1:8080';
	$scope.showSnapShot = false;
	$scope.data = {
			duration: 4,
			left: 12,
			right: 13,
			direction: 'center',
			url: 'http://10.3.141.165:8888',
			cannyLowThreshold: 50,
			cannyHighThreshold: 150,
			houghThreshold: 15,
			houghMinLineLength: 20,
			houghMaxLineGap: 20
	};
    this.startStop = function() {
		$http.get('webservice.php')
			.then(function(response) {
				$scope.response = response;
			}, function(response) {
				$scope.response = response;
			});
	};
    this.perform = function(direction) {
    	$scope.data.direction = direction;
    	console.log("perform: ", $scope.data);
		$http.post(host + '/perform', $scope.data)
			.then(function(response) {
				console.log("done: ", response);
				$scope.response = response.data;
			});
	};
    this.streamVideo = function() {		
		$scope.showSnapShot = false;
		$scope.videoUrl = $scope.data.url + '/video';
		$http.post(host + '/videoOnOff', $scope.data)
			.then(function(response) {
				$scope.videoUrl = host + '/video';
				$scope.response = response;			
			}, function(response) {
				$scope.response = response;			
			});
	};
    this.snapShot = function() {
		const start = new Date().getTime();
		$http.post(host + '/prepareImage', $scope.data)
			.then(function (response) {
				const end = new Date().getTime();
				// add the date to the end of the image ensure the broser to reload the image
				$scope.showSnapShotImage = 'tmp/snapShot.jpg?t=' + end; 
				$scope.showSnapShot = true;
				$scope.response = response;			
			}, function(response) {
				$scope.response = response;			
			});	
	};

    console.log("initialize controller");
});
