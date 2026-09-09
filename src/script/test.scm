; recursive factorial implementation
(define factorial
	(lambda (n)
		(if (= n 0)
			1
			(* n (factorial (- n 1))))))

; evaluate factorial of 6
(define result (factorial 6))
