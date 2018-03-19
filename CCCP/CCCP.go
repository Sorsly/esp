package main

import (
	"log"
	"net"
	"net/http"
	"runtime"
	"sync"
	"time"
)

const NUMBOTS = 1
const OUTPORT = "1917"
const CAMPORT = "1918"

func main() {
	runtime.GOMAXPROCS(10)
	ips := getConfig("ips.txt")
	host := ips.Cpu
	log.Println(host)
	inportstart := 2000
	var commandwg sync.WaitGroup

	// String to communicate out with bots
	outServerAddr, err := net.ResolveUDPAddr("udp", net.JoinHostPort(host, OUTPORT))
	CheckError(err)

	//Initializing camera
	cam := initcamera(NUMBOTS, CAMPORT)

	//Initializing sheep connections
	sheeps := make([]*Sheep, len(ips.Bot))
	for i, ip := range ips.Bot {
		sheeps[i] = initsheep(ip, host, uint16(inportstart+i))
	}

	//IDing process
	for i, sheep := range sheeps {
		sheep.commands.sheepF |= SHEEPRST
		sheep.commands.sheepF |= SHEEPLIGHT
		sheep.sendCommands(outServerAddr)

		wait := time.NewTimer(time.Millisecond * 100)
		<-wait.C

		ids, xs, ys := cam.getPos()
		sheep.idnum = ids[i]
		sheep.currX = xs[i]
		sheep.currY = ys[i]
	}

	//Initializing Frontend Server
	datawrite := MkChanDataWrite(100, 5)
	http.HandleFunc("/", http.HandlerFunc(datawrite.APIserve))
	go http.ListenAndServe(numtoportstr(80), nil)

	gamedone := false
	for gamedone == false {
		//DO FRONT END COMMUNICATION STUFF (HERE IS WHERE GAMEDONE IS CHECKED)
		ids, xs, ys := cam.getPos()
		for _, sheep := range sheeps {
			found := false
			for i, id := range ids {
				if sheep.idnum == id {
					sheep.currX = xs[i]
					sheep.currY = ys[i]
					found = true
					break
				}
			}
			if !found {
				log.Println(sheep.idnum, sheep.currX, sheep.currY)
				panic("WE HAVE LOST A SHEEP!\nLast Position is above")
			}
		}

		//FIND OUT THE PATH EVERYONE IS TAKING
		datawrite.FE1.UpdateGndBots(sheeps, false, false)

		//BREAK PATH INTO COMMANDS
		commandwg.Add(NUMBOTS)
		for _, sheep := range sheeps {
			go sheep.recState(&commandwg)
			wait := time.NewTimer(time.Nanosecond * 500)
			<-wait.C
			sheep.sendCommands(outServerAddr)
		}
		commandwg.Wait()
		log.Print(sheeps[0])

	}

	log.Println("GAME COMPLETE")
}
