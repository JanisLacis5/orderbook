query format: `{ACTION} {ACTION ARGS}`
output: 
cli flags: 

# Actions (for args, order is important)
- ADD {MARKET | GTC | GTE | FOK | FAK}
    args: quantity, side, price
- CANCEL
    args: orderId
- MODIFY
    args: orderId, modifications...
