: cr 10 emit
: negate 0 swap -
: if ' not ' branch? cr .s 2 0 cr .s depth >r cr .s immediate
: then depth r> - dup 2/ swap cr .s pick! cr .s immediate
: foo if 222 333 then 444 555

