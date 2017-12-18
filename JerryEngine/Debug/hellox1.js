function Person(name,age){
  this.name = name;
  this.age = age;
}

Person.prototype.sayInfo = function(){
  print("Name:" + this.name + " Age:" + this.age);
}

var person1 = new Person("Garry",40);
var person2 = new Person("Jackson",9);
var person3 = new Person("Sophia",1);

person1.sayInfo();
person2.sayInfo();
person3.sayInfo();
