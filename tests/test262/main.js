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

	if (test.path == "TestCases/ch07/7.8/7.8.4/7.8.4-1-s.js")
		return;

	if (test.path == "TestCases/ch07/7.8/7.8.4/7.8.4-10-s.js")
		return;

	if (test.path == "TestCases/ch07/7.8/7.8.4/7.8.4-11-s.js")
		return;


	try {
		eval("function myTestCase(){"+test.code+"};myTestCase()");
	} catch (e) {
		myTestException = e;
	}

	console.log("TEST: %s", test.description);

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

	if (fail)
		throw Error("test failed");
}
