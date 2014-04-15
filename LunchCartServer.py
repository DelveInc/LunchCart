###################################################################
# LunchCartServer.py
#  written by Jack Boland
# 
# Portions of socket code modified from: 
#	http://www.tutorialspoint.com/python/python_networking.htm
#
###################################################################

import smtplib
import socket
import time

# Email Settings
content = "The Lunch Cart is on the move."	# Establish what the email will say
recipient = "jcboland91@gmail.com"		# Determine who will receive the email
sender = "DCILunchCart@gmail.com"		# Send email from this address
password = "***********"			# Password of the sending email (redacted)

def sendEmail(emailAddr):
	mail = smtplib.SMTP('smtp.gmail.com', 587)
	mail.ehlo()
	mail.starttls()
	mail.login(sender, password)
	mail.sendmail(sender, emailAddr, content)

	# Confirm that the message was sent
	print("Sent")
	mail.close()

def checkLevel(incoming):
	incoming = int(float(incoming))
	if (incoming == 1):
		print("Send Email")
		sendEmail("jack.boland@design-concepts.com")


input = '0'							# Incoming message

# Open up a socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)	# Create a socket object
host = socket.gethostname()					# Get local machine name
port = 	12345	 						# Reserve a port
ip = socket.gethostbyname(socket.gethostname())
print(ip)
s.bind((host, port))					# Bind to the port

s.listen(5)						# Now wait for client connection
while True:
	msg = 'Thank you for connecting'
	c, addr = s.accept()				# Establish connection with client
	print ('Got Connection from ', addr)		# Spits back IP Address of client
	c.send(msg.encode('ASCII'))
	
	while True:
		data = str(c.recv(1024), 'ASCII')
		checkLevel(data)

	print("Done Sleeping")
	c.send("Read this?".encode('ASCII'))
		
	c.close()					# Close the connection
	exit()
