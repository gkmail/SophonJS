function fail()
{
	throw Error("test failed");
}

function property_test()
{
	if ([].length != 0)
		fail();
	if ([,].length != 1)
		fail();
	if ([,,].length != 2)
		fail();
	if ([1].length != 1)
		fail();
	if ([1,].length != 1)
		fail();
	if ([1,,].length != 2)
		fail();
	if ([1,,,].length != 3)
		fail();
	if ([1,,,2].length != 4)
		fail();

	var a = Array();
	if (a.length != 0)
		fail();

	a.length = 10;
	if (a.length != 10)
		fail();

	a.length = 0;
	if (a.length != 0)
		fail();

	console.log("property test passed");
}

property_test();

function constructor_test()
{
	var a, i;

	a = Array();
	if (!Array.isArray(a) || (a.length != 0))
		fail();

	a = Array(10);
	if (!Array.isArray(a) || (a.length != 10))
		fail();
	for (i = 0; i < 10; i++)
		if (a[i] != undefined)
			fail();

	a = Array(1,2);
	if (!Array.isArray(a) || (a.length != 2))
		fail();
	if (a[0] != 1)
		fail();
	if (a[1] != 2)
		fail();

	var str = "Array(";
	for (i = 0; i < 255; i++) {
		str += i;
		if (i == 254)
			str += ")";
		else
			str += ",";
	}
	a = eval(str);
	if (!Array.isArray(a) || (a.length != 255))
		fail();
	for (i = 0; i < 255; i++)
		if (a[i] != i)
			fail();

	console.log("constructor test passed");
}

constructor_test();

function array_equ(a1, a2)
{
	var i;

	if (a1.length != a2.length)
		return false;

	for (i = 0; i < a1.length; i++) {
		if (a1[i] != a2[i])
			return false;
	}

	return true;
}

function prototype_test()
{
	var a, r, str, i, j;

	a = [1,2,3,4];
	if (a.toString() != "1,2,3,4")
		fail();

	a = [1,,,4];
	if (a.toString() != "1,,,4")
		fail();

	a = [1,,,];
	if (a.toString() != "1,,")
		fail();

	a = [,,,];
	if (a.toString() != ",,")
		fail();

	a = [1,2,3];
	r = a.concat();
	if (!array_equ(a, [1,2,3]))
		fail();
	if (!array_equ(a, [1,2,3]))
		fail();

	r = a.concat(4);
	if (!array_equ(a, [1,2,3]))
		fail();
	if (!array_equ(r, [1,2,3,4]))
		fail();

	r = a.concat(4,5,6);
	if (!array_equ(r, [1,2,3,4,5,6]))
		fail();

	a = [1,2,3];
	str = a.join();
	if (str != "1,2,3")
		fail();

	str = a.join(" ");
	if (str != "1 2 3")
		fail();

	a = [1,,2,,3];
	str = a.join();
	if (str != "1,,2,,3")
		fail();

	a = Array();
	for (i = 0; i < 1024; i++)
		a[i] = i;
	for (i = 0; i < 1024; i++) {
		r = a.pop();
		if (r != 1023 - i)
			fail();
		if (a.length != 1023 - i)
			fail();
	}

	a = [1,2,3];
	if (a.push(4) != 4)
		fail();
	if (!array_equ(a, [1,2,3,4]))
		fail();
	if (a.push(5,6) != 6)
		fail();
	if (!array_equ(a, [1,2,3,4,5,6]))
		fail();
	if (a.push(7,8,9) != 9)
		fail();
	if (!array_equ(a, [1,2,3,4,5,6,7,8,9]))
		fail();

	a = Array();
	for (i = 0; i < 1024; i++)
		a[i] = i;
	if (a.reverse() != a)
		fail();
	for (i = 0; i < 1024; i++)
		if (a[i] != 1023 - i)
			fail();

	a = Array();
	for (i = 0; i < 1024; i++)
		a[i] = i;
	for (i = 0; i < 1024; i++) {
		r = a.shift();
		if (r != i)
			fail();
		if (a.length != 1023 - i)
			fail();
		for (j = 0; j < a.length; j++) {
			if (a[j] != i + j + 1)
				fail();
		}
	}

	a=[0,1,2,3,4,5,6,7,8,9];
	r = a.slice(0,5);
	if (!array_equ(r, [0,1,2,3,4]))
		fail();
	r = a.slice(5,9);
	if (!array_equ(r, [5,6,7,8]))
		fail();
	r = a.slice(5,-2);
	if (!array_equ(r, [5,6,7]))
		fail();
	r = a.slice(-8,-2);
	if (!array_equ(r, [2,3,4,5,6,7]))
		fail();
	r = a.slice(5,5);
	if (!array_equ(r, []))
		fail();
	r = a.slice(5,4);
	if (!array_equ(r, []))
		fail();

	a=[9,8,7,6,5,4,3,2,1,0];
	if (a.sort() != a)
		fail();
	if (!array_equ(a, [0,1,2,3,4,5,6,7,8,9]))
		fail();

	a=[1,2,3,11,22,33];
	a.sort();
	if (!array_equ(a, [1,11,2,22,3,33]))
		fail();
	a.sort(function(x,y){return x-y});
	if (!array_equ(a, [1,2,3,11,22,33]))
		fail();

	a=[undefined,undefined,3,1,4,,5,,2];
	a.sort();
	if (a[0] != 1)
		fail();
	if (a[1] != 2)
		fail();
	if (a[2] != 3)
		fail();
	if (a[3] != 4)
		fail();
	if (a[4] != 5)
		fail();
	if (a[5] != undefined)
		fail();
	if (a[6] != undefined)
		fail();
	if (a.hasOwnProperty(7))
		fail();
	if (a.hasOwnProperty(8))
		fail();

	a=[0,1,2,3,4,5,6,7,8,9];
	r = a.splice(0, 5);
	if (!array_equ(r, [5,6,7,8,9]))
		fail();
	r = a.splice(5,3);
	if (!array_equ(r, [0,1,2,3,4,8,9]))
		fail();
	r = a.splice(5,3,-5,-6,-7);
	if (!array_equ(r, [0,1,2,3,4,-5,-6,-7,8,9]))
		fail();
	r = a.splice(5,3,-100);
	if (!array_equ(r, [0,1,2,3,4,-100,8,9]))
		fail();
	r = a.splice(5,3,-100,-101,-102,-103);
	if (!array_equ(r, [0,1,2,3,4,-100,-101,-102,-103,8,9]))
		fail();
	r = a.splice(-3,2,10,20,30);
	if (!array_equ(r, [0,1,2,3,4,5,6,10,20,30,9]))
		fail();

	a=[0,1,2,3,4];
	r = a.unshift();
	if ((r != 5) || !array_equ(a, [0,1,2,3,4]))
		fail();
	r = a.unshift(-1);
	if ((r != 6) || !array_equ(a, [-1,0,1,2,3,4]))
		fail();
	r = a.unshift(-3,-2);
	if ((r != 8) || !array_equ(a, [-3,-2,-1,0,1,2,3,4]))
		fail();

	a=[0,1,2,3,4,5,0,1,2,3,4,5];
	if (a.indexOf(4) != 4)
		fail();
	if (a.indexOf(4,4) != 4)
		fail();
	if (a.indexOf(4,5) != 10)
		fail();
	if (a.indexOf(4,-2) != 10)
		fail();
	if (a.indexOf(4,-7) != 10)
		fail();
	if (a.indexOf(4,-8) != 4)
		fail();
	if (a.indexOf(6) != -1)
		fail();

	if (a.lastIndexOf(4) != 10)
		fail();
	if (a.lastIndexOf(4,10) != 10)
		fail();
	if (a.lastIndexOf(4,9) != 4)
		fail();
	if (a.lastIndexOf(4,-1) != 10)
		fail();
	if (a.lastIndexOf(4,-2) != 10)
		fail();
	if (a.lastIndexOf(4,-3) != 4)
		fail();
	if (a.lastIndexOf(6) != -1)
		fail();

	a=[0,10,20,30,40,50,60,70,80,90,100];

	j = 0;
	r = a.every(function(v,i,arr){
		if (i != j)
			fail();
		if (v != j*10)
			fail();
		if (arr != a)
			fail();
		j++;
		return true;
	});
	if (r != true)
		fail();
	if (j != 11)
		fail();

	j = 0;
	r = a.every(function(v,i,arr){
		if (i != j)
			fail();
		if (v != j*10)
			fail();
		if (arr != a)
			fail();
		j++;
		if (j == 6)
			return false;
		return true;
	});
	if (r != false)
		fail();
	if (j != 6)
		fail();

	j = 0;
	r = a.every(function(v,i,arr){
		if (i != j)
			fail();
		if (v != j*10)
			fail();
		if (arr != a)
			fail();
		j++;
		if (j == this)
			return false;
		return true;
	}, 8);
	if (r != false)
		fail();
	if (j != 8)
		fail();

	j = 0;
	r = a.some(function(v,i,arr){
		if (i != j)
			fail();
		if (v != j*10)
			fail();
		if (arr != a)
			fail();
		j++;
		return false;
	});
	if (r != false)
		fail();
	if (j != 11)
		fail();

	j = 0;
	r = a.some(function(v,i,arr){
		if (i != j)
			fail();
		if (v != j*10)
			fail();
		if (arr != a)
			fail();
		j++;
		if (j == 6)
			return true;
		return false;
	});
	if (r != true)
		fail();
	if (j != 6)
		fail();

	j = 0;
	r = a.some(function(v,i,arr){
		if (i != j)
			fail();
		if (v != j*10)
			fail();
		if (arr != a)
			fail();
		j++;
		if (j == this)
			return true;
		return false;
	}, 8);
	if (r != true)
		fail();
	if (j != 8)
		fail();

	j = 0;
	a.forEach(function(v,i,arr){
		if (i != j)
			fail();
		if (v != j*10)
			fail();
		if (arr != a)
			fail();
		j++;
		return false;
	});
	if (j != 11)
		fail();

	j = 0;
	a.forEach(function(v,i,arr){
		if (i != j)
			fail();
		if (v != j*10)
			fail();
		if (arr != a)
			fail();
		j++;
		if (j == 6)
			return true;
		return false;
	});
	if (j != 11)
		fail();

	j = 0;
	a.forEach(function(v,i,arr){
		if (i != j)
			fail();
		if (v != j*10)
			fail();
		if (arr != a)
			fail();
		if (this != 10)
			fail();
		j++;
	}, 10);
	if (j != 11)
		fail();

	r = a.map(function(v,i,arr){
		return -v;
	});
	if (!array_equ(r, [0,-10,-20,-30,-40,-50,-60,-70,-80,-90,-100]))
		fail();
	r = a.map(function(v,i,arr){
		return v+this;
	},9);
	if (!array_equ(r, [9,19,29,39,49,59,69,79,89,99,109]))
		fail();

	r = a.filter(function(v,i,arr){
		return i%2;
	});
	if (!array_equ(r, [10,30,50,70,90]))
		fail();
	r = a.filter(function(v,i,arr){
		return i%this;
	},3);
	if (!array_equ(r, [10,20,40,50,70,80,100]))
		fail();

	r = a.reduce(function(prev,v,i,arr){
		return prev+v;
	});
	if (r != 550)
		fail();

	r = a.reduce(function(prev,v,i,arr){
		return prev+v;
	},1000);
	if (r != 1550)
		fail();

	a = [0,1,2,3,4,5];
	r = a.reduce(function(prev,v,i,arr){
		return ""+prev+v;
	});
	if (r != "012345")
		fail();
	r = a.reduceRight(function(prev,v,i,arr){
		return ""+prev+v;
	});
	if (r != "543210")
		fail();

	console.log("prototype test passed");
}

prototype_test();

console.log("all tests passed");
