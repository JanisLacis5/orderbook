query format: `{ACTION} {ACTION ARGS}`
output: 
cli flags: 

# Actions (for args, order is important)
- ADD 
    args: orderType, quantity, side, price
    orderTypes: MARKET | GTC | GTE | FOK | FAK
- CANCEL
    args: orderId
- MODIFY
    args: orderId, modifications...
    modifications are in form `arg=val` where args can be: orderType, quantity, side, price. no spaces around `=`, does not work with spaces
