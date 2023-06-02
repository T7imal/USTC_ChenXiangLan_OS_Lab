.text
.code32 

#上下文切换
#上下文示意图：
#（高地址）CS：IP
#（低地址）
  .global CTX_SW
CTX_SW:  
	pushf
	pusha

	movl prevTASK_StackPtr, %eax
	movl %esp, (%eax)
	movl nextTASK_StackPtr, %esp

	popa 
	popf
	ret
