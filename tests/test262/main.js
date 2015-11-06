function myTest262(json)
{
	console.log("%s", json.testsCollection.name);
	json.testsCollection.tests.forEach(function(test){
		test.code = decode(test.code);
		myRunTest(test);
	})
}

var ignore = [
"TestCases/ch08/8.5/8.5.1.js",
"TestCases/ch08/8.5/S8.5_A11_T1.js",
"TestCases/ch08/8.5/S8.5_A13_T2.js",
"TestCases/ch08/8.5/S8.5_A2.1.js",
"TestCases/ch08/8.5/S8.5_A2.2.js"
];

function myRunTest(test)
{
	var myTestException = undefined;

	if (ignore.indexOf(test.path) != -1)
		return;

	myTestException = runTest(test.code);

	console.log("TEST:%s", test.path);
	console.log("%s", test.description);

	var fail = false;

	if (test.negative != undefined) {
		var neg = test.negative === "NotEarlyError" ? test.negative :
			(test.negative === "^((?!NotEarlyError).)*$" ?
            test.negative : ".");

		if (myTestException == undefined) {
			console.log("FAILED: cannot get excetpion '%s'", neg);
			fail = true;
		} else if (!(new RegExp(neg, "i").test(myTestException))) {
			console.log("FAILED: expect exception '%s', but get '%s'", neg, myTestException);
			fail = true;
		}
	} else {
		if (myTestException != undefined) {
			console.log("FAILED: get exception '%s'", myTestException);
			fail = true;
		}
	}

	if (fail) {
		dump(test.code);
	} else {
		console.log("PASSED");
	}

	if (fail) {
		throw Error("test failed");
	}
}
