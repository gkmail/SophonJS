function myTest262(json)
{
	console.log("%s", json.testsCollection.name);
	json.testsCollection.tests.forEach(function(test){
		test.code = decode(test.code);
		myRunTest(test);
	})
}

function myRunTest(test)
{
	var myTestException = undefined;

	//if (test.path != "TestCases/ch08/8.12/8.12.5/8.12.5-3-b_1.js")
	//	return;

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
