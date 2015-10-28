function fail()
{
	throw Error("test failed");
}

function property_test()
{
	function t1()
	{
	}

	if (t1.length != 0)
		fail();

	function t2(x)
	{
	}

	if (t2.length != 1)
		fail();

	function t3(x,y)
	{
	}

	if (t3.length != 2)
		fail();

	console.log("property test passed");
}

property_test();

function call_test()
{
	var o = {
		p:1,
		func: function(x,y){
			return this.p+x+y;
		}
	}

	if (o.func(10, 100) != 111)
		fail();
	if (o.func.call({p:"a"},"b","c") != "abc")
		fail();
	if (o.func.apply({p:"sophon"},[" ","js"]) != "sophon js")
		fail();

	console.log("call test passed");
}

call_test();

function arguments_test()
{
	function test1(x,y)
	{
		return arguments.length;
	}

	if (test1() != 0)
		fail();
	if (test1(1) != 1)
		fail();
	if (test1(1,2) != 2)
		fail();
	if (test1(1,2,3,4,5) != 5)
		fail();

	function test2(x,y,z)
	{
		arguments[0] = "new0";
		arguments[1] = "new1";
		arguments[2] = "new2";

		if (x != "new0")
			fail();
		if (y != "new1")
			fail();
		if (z != "new2")
			fail();
	}

	test2(0,1,2);
	test2(0);

	function test3(x,y,z)
	{
		if (arguments.callee.length != 3)
			fail();
	}

	test3();

	function test4(x,y)
	{
		if (arguments.caller.length !=3)
			fail();
		if (arguments.caller.arguments.length != 5)
			fail();
	}
	function test5(x,y,z)
	{
		test4(x,y);
	}

	test5(1,2,3,4,5);

	console.log("arguments test passed");
}

arguments_test();

function bind_test()
{
	function test(x,y,z)
	{
		return x+y+z;
	}

	var bind = test.bind(undefined, 10, 100, 1000);
	if (bind() != 1110)
		fail();

	var o = {
		p: 1,
		func: function(x,y){
			return this.p+x+y;
		}
	}

	o.p = 1978;
	if (o.func(10000,100000) != 111978)
		fail();

	var bo = Object;
	bo.p = 10;
	bind = o.func.bind(bo, 100, 1000);
	if (bind() != 1110)
		fail();

	console.log("bind test passed");
}

bind_test();

console.log("all tests passed");
