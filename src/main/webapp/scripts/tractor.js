angular.module('tractor',[])
.controller('TractorController', function($scope, $http) {
	$scope.showSnapShot = false;
	$scope.data = {
			duration: 4,
			left: 0.25,
			center: 0.5,
			right: 1,
			direction: 'center',
			url: 'http://10.3.141.165:8888/',
			cannyLowThreshold: 50,
			cannyHighThreshold: 150,
			houghThreshold: 15,
			houghMinLineLength: 20,
			houghMaxLineGap: 20
	};
	
    this.perform = function(direction) {
		$scope.showSnapShot = false;
    	$scope.data.direction = direction;
    	console.log("perform: ", $scope.data);
		$http.post('tractor.php', $scope.data).then(function(response) {
	    	console.log("done: ", response);
	        $scope.response = response.data;
		});
	};
    this.startVideo = function() {
		$scope.showSnapShot = false;
	};
    this.snapShot = function() {		
		$http.post('snapShot.php', $scope.data).then(function(response) {
			$scope.response = response.data;
			if (!$scope.response.startsWith('nolines')) {
				$scope.showSnapShotImage = response.data.substring(0,44);
				$scope.showSnapShot = true;
			} else {
				console.log("show video because no lines where found");
				$scope.showSnapShot = false;
			}
			
		});
	};

    console.log("initialize controller");
});
