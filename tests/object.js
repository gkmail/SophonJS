function fail()
{
	throw Error("test failed");
}

function constructor_test()
{
	var r, o;

	r = Object();
	if (!(r instanceof Object))
		fail();

	r = Object(1);
	if (!(r instanceof Object) || !(r instanceof Number) || (r != 1))
		fail();

	r = Object("sophon");
	if (!(r instanceof Object) || !(r instanceof String) || (r != "sophon"))
		fail();

	r = Object(true);
	if (!(r instanceof Boolean) || !(r instanceof Boolean) || (r != true))
		fail();

	o = Object();
	r = Object(o);
	if (!(r instanceof Object) || (r != o))
		fail();

	r = new Object();
	if (!(r instanceof Object))
		fail();

	r = new Object(1);

	if (!(r instanceof Object) || !(r instanceof Number) || (r != 1))
		fail();

	r = new Object("sophon");
	if (!(r instanceof Object) || !(r instanceof String) || (r != "sophon"))
		fail();

	r = new Object(true);
	if (!(r instanceof Boolean) || !(r instanceof Boolean) || (r != true))
		fail();

	o = new Object();
	r = new Object(o);
	if (!(r instanceof Object) || (r != o))
		fail();

	console.log("constructor test passed");
}

constructor_test();

function function_test()
{
	var r, o, p, v, i;

	function constructor(){
		this.p1=1;
		this.p2=2;
		this.p3=3;
	}

	var prototype = {
		protoFunc: function(){
			return "proto";
		}
	};

	constructor.prototype = prototype;

	r = 0;
	try{
		Object.getPrototypeOf(1);
	}catch(e){
		if (e instanceof(TypeError))
			r = 1;
	}
	if (r != 1)
		fail();

	o = new constructor();
	if (o.protoFunc() != "proto")
		fail();
	if (Object.getPrototypeOf(o) != prototype)
		fail();

	p = Object.getOwnPropertyDescriptor(o, "pp");
	if (p != void 0)
		fail();

	function check(name, v) {
		p = Object.getOwnPropertyDescriptor(o, name);
		if (p.value != v)
			fail();
		if (!p.writable)
			fail();
		if (!p.configurable)
			fail();
		if (!p.enumerable)
			fail();
	}
	check("p1", 1);
	check("p2", 2);
	check("p3", 3);

	r = Object.getOwnPropertyNames(o);
	if (r.length != 3)
		fail();
	if (r.indexOf("p1") == -1)
		fail();
	if (r.indexOf("p2") == -1)
		fail();
	if (r.indexOf("p3") == -1)
		fail();

	Object.defineProperty(o, "p4", {
		value:19781009,
		writable:false,
		enumerable:false,
		configurable:false
	});
	if (o.p4!=19781009)
		fail();
	r = Object.getOwnPropertyDescriptor(o, "p4");
	if (r.writable)
		fail();
	if (r.enumerable)
		fail();
	if (r.configurable)
		fail();

	Object.defineProperty(o, "p5", {
		get: function() {
			return 19491009;
		},
		enumerable:true,
		configurable:true
	});
	if (o.p5 != 19491009)
		fail();
	r = Object.getOwnPropertyDescriptor(o, "p5");
	if (!r.enumerable)
		fail();
	if (!r.configurable)
		fail();

	Object.defineProperties(o,{
		x1:{
			value:"hello"
		},
		x2:{
			value:"world"
		}
	});
	if (o.x1 != "hello")
		fail();
	if (o.x2 != "world")
		fail();

	o = Object.create(prototype, {
		p1:{
			value:1,
			writable:true
		},
		p2:{
			value:2
		},
		accessor:{
			get:function(){
				return this.p1+100;
			},
			set:function(v){
				this.p1=v;
			}
		}
	})
	if (o.p1!=1)
		fail();
	if (o.p2!=2)
		fail();
	if (o.accessor!=101)
		fail();
	o.accessor=88;
	if (o.accessor!=188)
		fail();

	o=Object();
	o.p=1;
	if (o.p != 1)
		fail();
	o.p=2;
	if (o.p != 2)
		fail();

	Object.defineProperty(o,"p",{writable:false});
	o.p=2;
	r = 0;
	try{
		o.p=1;
	}catch(e){
		if (e instanceof TypeError)
			r = 1;
	}
	if (r != 1)
		fail();

	o=Object();
	o.p=1;
	if (Object.isSealed(o))
		fail();
	Object.seal(o);
	if (!Object.isSealed(o))
		fail();
	r = 0;
	try{
		o.p1=2;
	}catch(e){
		if (e instanceof TypeError)
			r = 1;
	}
	if (r != 1)
		fail();
	o.p=1978;
	r = 0;
	try{
		Object.defineProperty(o,"p",{writable:false});
	}catch(e){
		if (e instanceof TypeError)
			r = 1;
	}
	if (r != 1)
		fail();

	o = Object();
	o.p=1;
	if (Object.isFrozen(o))
		fail();
	Object.freeze(o);
	if (!Object.isFrozen(o))
		fail();
	r = 0;
	try{
		o.p1=2;
	}catch(e){
		if (e instanceof TypeError)
			r = 1;
	}
	if (r != 1)
		fail();
	r = 0;
	try{
		o.p=1978;
	}catch(e){
		if (e instanceof TypeError)
			r = 1;
	}
	if (r != 1)
		fail();
	r = 0;
	try{
		Object.defineProperty(o,"p",{enumerable:false});
	}catch(e){
		if (e instanceof TypeError)
			r = 1;
	}
	if (r != 1)
		fail();

	o = Object();
	o.p = 1;
	if (!Object.isExtensible(o))
		fail();
	Object.preventExtensions(o);
	if (Object.isExtensible(o))
		fail();
	o.p=1978;
	Object.defineProperty(o,"p",{enumerable:false});
	r = 0;
	try {
		o.p1=1;
	}catch(e){
		if (e instanceof TypeError)
			r = 1;
	}
	if (r != 1)
		fail();

	o = Object();
	for (i = 0; i < 1024; i++) {
		eval("o.p"+i+"=1");
	}
	r = Object.keys(o);
	if (r.length != 1024)
		fail();
	for (i=0; i<1024; i++){
		if (r.indexOf("p"+i) == -1)
			fail();
	}
	Object.defineProperty(o, "p666", {enumerable:false});
	r = Object.keys(o);
	if (r.length != 1023)
		fail();
	if (r.indexOf("p666") != -1)
		fail();

	console.log("function test passed");
}

function_test();

function prototype_test()
{
	var r, o;

	function constructor(){
	}

	var prototype = {
	};

	function constructor2(){
	}

	var prototype2 = {
	};

	constructor.prototype = prototype;
	constructor2.prototyep = prototype2;

	o = new constructor();
	if (!prototype.isPrototypeOf(o))
		fail();
	if (!Object.prototype.isPrototypeOf(o))
		fail();
	if (prototype2.isPrototypeOf(o))
		fail();

	o = Object();
	if (o.hasOwnProperty("p1"))
		fail();
	o.p1=1;
	if (!o.hasOwnProperty("p1"))
		fail();
	if (!o.propertyIsEnumerable("p1"))
		fail();
	Object.defineProperty(o, "p1", {enumerable:false});
	if (o.propertyIsEnumerable("p1"))
		fail();

	function tostr(o, str)
	{
		if (Object.prototype.toString.call(o) != str)
			fail();
	}

	tostr(null, "[object Null]");
	tostr(undefined, "[object Undefined]");
	tostr("", "[object String]");
	tostr(true, "[object Boolean]");
	tostr(1, "[object Number]");
	tostr([], "[object Array]");
	tostr({}, "[object Object]");

	console.log("prototype test passed");
}

prototype_test();

console.log("all tests passed");
