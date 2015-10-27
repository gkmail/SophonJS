function fail()
{
	throw Error("test failed");
}

function const_test()
{
	function test(name)
	{
		console.log("%s:%s", name, eval("Math."+name));
	}

	test("E");
	test("LN10");
	test("LN2");
	test("LOG2E");
	test("LOG10E");
	test("PI");
	test("SQRT1_2");
	test("SQRT2");

	console.log("const test passed");
}

const_test();

function min_max_test()
{
	function test(v, r)
	{
		if (v != r)
			fail();
	}

	test(Math.max(),-Infinity);
	test(Math.max(-100),-100);
	test(Math.max(-100,99,98,97,77,0,1),99);

	test(Math.min(),Infinity);
	test(Math.min(-100),-100);
	test(Math.min(-100,99,98,97,77,0,1,-99),-100);


	console.log("min max test passed");
}

min_max_test();

function random_test()
{
	var i;

	for (i = 0; i < 100; i++) {
		var r = Math.random();
		console.log("random: %f", r);
	}
	console.log("random test passed");
}

random_test();

function function_test()
{
	function test(v, r)
	{
		if (v != r)
			fail();
	}

	test(Math.abs(0),0);
	test(Math.abs(1),1);
	test(Math.abs(-1),1);
	test(Math.acos(1),0);
	test(Math.asin(0),0);

	console.log("function test passed");
}

function_test();

console.log("all tests passed");
