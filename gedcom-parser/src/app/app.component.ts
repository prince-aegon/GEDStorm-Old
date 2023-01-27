import { Component, OnInit } from '@angular/core';
import indiData from '../assets/indi.json';

interface Individual {
  id: string;
  name: string;
  year: number;
}

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css'],
})
export class AppComponent implements OnInit {
  title = 'gedcom-parser';
  individuals: any;
  keys: any;

  ngOnInit(): void {
    const x = JSON.stringify(indiData);
    const y = JSON.parse(x);
    console.log(y);
    // for (var i = 0; i < 2; i++) console.log(y['@I0001@'].name);
    this.individuals = y;
    this.keys = Object.keys(y);
    for (let key in this.individuals) {
      console.log(this.individuals[key].name);
    }
    console.log(x);
  }
}
