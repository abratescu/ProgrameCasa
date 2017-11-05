#!/usr/bin/python

from __future__ import print_function
import MySQLdb

def getTemps(tabel,nrTemps,noPrint):
	db = MySQLdb.connect("localhost","andy","1234","casa" )
	cursor = db.cursor()
	sql = "SELECT * FROM "
	sql += tabel
	sql += " ORDER BY timp DESC LIMIT 1"	
	cursor.execute(sql)
	results = cursor.fetchall()
	if noPrint:
		return results
		db.close()
	for row in results:
		for i in range(0,nrTemps):
			if i != nrTemps-1:
				print(row[i],end=", ")
			else:
				print(row[i])
	db.close()
	return results
def getWantedTemps(noPrint):
	#ia tempDorita din baza de date
	db = MySQLdb.connect("localhost","andy","1234","casa" )
	cursor = db.cursor()
	sql = "SELECT * FROM setari"	
	cursor.execute(sql)
	results = cursor.fetchall()
	if noPrint:
		db.close()
		return results[0][0]
	print("tempDorita=",end="")
	print(results[0][0],end="\n")#la [0][0] in structura e tempDorita 
	db.close()
	return results[0][0]
def releePanou(tempPanou,tempDorita):
	"""
	SfrCentru - RIndex:0 - TIndex 1
	SfrStanga - RIndex:1 - TIndex 0
	SfrFerestra - RIndex:2 - TIndex 2
	Intrare - RIndex:3 - TIndex12 
	SufrDreapta - RIndex:5 - TIndex 3
	"""
	releeP=list("releu00000000")
	mapRT={ 0:1, 1:0, 2:2, 5:3, } # Nu e pusa Intrarea
	offset=5
	for i in mapRT:
		if conditieTemp(tempPanou[mapRT[i]],tempDorita):
			releeP[offset+i]='1'
	return (releeP)
def releeScara(tempPanou,tempDorita):
	"""
	cada - RIndex:0 - TIndex 6	
	baie - RIndex:1 - TIndex 6	
	buc1 - RIndex:2 - TIndex 7	
	buc3 - RIndex:5 - TIndex 9	
	atelier - RIndex:3 - TIndex 10
	birou - RIndex:4 - TIndex 11
	"""
	releeScara=list("releu00000000")
	mapRT={1:6 ,2:7 ,5:9 , 4:11}# nu e casa si atelier
	offset=5
	for i in mapRT:
		if conditieTemp(tempPanou[mapRT[i]],tempDorita):
			releeScara[offset+i]='1'
	return (releeScara)
def releeCentrala(tempStefan,tempAndy,releePanou,releeScara,tempDorita):
	"""
	PompaCentrala - RIndex:5
	PompaHol - RIndex:6
	PompaScara - RIndex:7
	"""
	releeCentrala=list("releu00000000")
	mapRT={}
	releePornite=0
	releePorniteScara=0
	releePorniteHol=0
	for i in range(5,13):
		if releePanou[i]=='1':
			releePorniteHol+=1
	for i in range(5,13):
		if releeScara[i]=='1':
			releePorniteScara+=1
	offset=5
	releePornite=releePorniteHol+releePorniteScara
	if releePornite>=2: #defineste asta undeva
		releeCentrala[offset+5]='1'
	if releePorniteHol >=2 or tempAndy[0][0]-8 < tempDorita or tempStefan[0][0]-8 < tempDorita:
		releeCentrala[offset+6]='1'
	if releePorniteScara >=2:
		releeCentrala[offset+7]='1'
	return releeCentrala

def conditieTemp(temp,tempDorita):
	if temp<tempDorita:
		return True
	return False
def getRelee(noPrint):
	tempPanou=getTemps("panou",12,noPrint)                                                                                                                            
	tempA=getTemps("andy",1,noPrint)                                                                                                                                  
	tempS=getTemps("andy",1,True) #hardcodat                                                                                                                                   
	tempDorita=getWantedTemps(noPrint)                                                                                                                                
	releeP=releePanou(tempPanou[0],tempDorita)                                                                                                                      
	releeS=releeScara(tempPanou[0],tempDorita)
	releeC=releeCentrala(tempS,tempA,releeP,releeS,tempDorita)
	if noPrint:	
		return (releeP,releeS,releeC)
	else:
		print(releeP)
		print(releeS)
		print(releeC)
		return (releeP,releeS,releeC)
