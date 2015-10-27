function fail()
{
	throw Error("test failed")
}

function constructor_test()
{
	function test_true(v)
	{
		if (v !== true)
			fail();
	}

	function test_false(v)
	{
		if (v !== false)
			fail();
	}

	test_false(Boolean());
	test_false(Boolean(false));
	test_false(Boolean(0));
	test_false(Boolean(null));
	test_false(Boolean(undefined));
	test_false(Boolean(""));
	test_false(Boolean(new Boolean(false)));
	test_true(Boolean(true));
	test_true(Boolean(1));
	test_true(Boolean("s"));
	test_true(Boolean(new Object()));
	test_true(Boolean(new Boolean(true)));

	var r;

	r = new Boolean();
	if (r != false)
		fail();
	if (!(r instanceof Object))
		fail();

	r = new Boolean(false);
	if (r != false)
		fail();
	if (!(r instanceof Object))
		fail();

	r = new Boolean(true);
	if (r != true)
		fail();
	if (!(r instanceof Object))
		fail();

	r = new Boolean("");
	if (r != false)
		fail();
	if (!(r instanceof Object))
		fail();

	console.log("constructor test passed");
}

constructor_test();

function prototype_test()
{
	if (true.toString() != "true")
		fail();
	if (false.toString() != "false")
		fail();

	if (true === new Boolean(true))
		fail();
	if (true !== (new Boolean(true)).valueOf())
		fail();

	if (false === new Boolean(false))
		fail();
	if (false !== (new Boolean(false)).valueOf())
		fail();

	console.log("prototype test passed");
}

prototype_test();

console.log("all test passed");
