//Test of JerryScript engine under HelloX.

//Fit for node.js,since print routine is not implemented.
if(typeof(print) == 'undefined')
{
    var print = function(str){
        console.log(str);
    }
}

print("JerryScript under HelloX.");

function TreeNode(value){  
    this.value = value;  
      
}  
function makeBinaryTreeByArray(array,index){  
    if(index<array.length){  
        var value=array[index];  
        if(value!=0){  
            var t=new TreeNode(value);  
            array[index]=0;  
            t.left=makeBinaryTreeByArray(array,index*2);  
            t.right=makeBinaryTreeByArray(array,index*2+1);  
            return t;  
        }  
    }  
    return null;  
}  
  
function BinaryTree(ary){  
    this.left = new TreeNode(),  
    this.right = new TreeNode(),  
    this.root = makeBinaryTreeByArray(ary,1);  
      
      
}  
// 广度优先遍历  
BinaryTree.prototype.levelOrderTraversal = function(){  
    if(this.root==null){  
        print("empty tree");  
        return;  
    }  
    var queue = [];  
    queue.push(this.root);  
    while(queue.length!==0){  
        var node=queue.pop();  
        print('wid' + ' ' + node.value+"    ");  
        if(node.left!=null){  
            queue.push(node.left);  
        }  
        if(node.right!=null){  
            queue.push(node.right);  
        }  
    }  
    print("\n");  
};  
// 深度优先遍历  
BinaryTree.prototype.depthOrderTraversal = function(){  
        if(this.root==null){  
            print("empty tree");  
            return;  
        }         
        var stack=[];  
        stack.push(this.root);         
        while(stack.length > 0){  
            var node=stack.pop();  
            print('deep' + ' ' + node.value+"    ");  
            if(node.right!=null){  
                stack.push(node.right);  
            }  
            if(node.left!=null){  
                stack.push(node.left);  
            }             
        }  
    print("\n");  
};  
var arr=[0,13,65,5,97,25,0,37,22,0,4,28,0,0,32,0];  
print(arr);
var tree=new BinaryTree(arr);  
for(var i = 0;i < 1024;i ++)
{
    tree.levelOrderTraversal();  
    tree.depthOrderTraversal();  
    print("Round " + i + " over.");
}
