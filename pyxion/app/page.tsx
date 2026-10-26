"use client";

import { useState } from "react";
import Dropdown from "./Dropdown";
import { Luckiest_Guy } from "next/font/google";

const luckiest = Luckiest_Guy({ weight: "400", subsets: ["latin"] });

const MONTHS = [
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December",
];
const DAYS = Array.from({ length: 31 }, (_, i) => String(i + 1));
const YEARS = Array.from({ length: 100 }, (_, i) => String(new Date().getFullYear() - i));

export default function Home() {
  const [month, setMonth] = useState("");
  const [day, setDay] = useState("");
  const [year, setYear] = useState("");
  const [gender, setGender] = useState<"male" | "female" | "">("");
  return (
    <div className="flex flex-col min-h-auto font-sans bg-[#000]">

      <nav className="h-[10vh] w-full bg-[#393b3d] text-white px-[10px] flex flex-row items-center gap-[14px] font-sans">
        <h2 className="mr-auto font-display text-4xl font-bold text-white [-webkit-text-stroke:1px_#f03f00]">Pyxion</h2>
        <button className="font-bold hover:underline cursor-pointer">Home</button>
        <button className="font-bold hover:underline cursor-pointer">About</button>
        <button className="font-bold hover:underline cursor-pointer">Contact</button>
        <button className="font-bold hover:underline cursor-pointer">Create Account</button>
        <button className="bg-[#02b757] hover:bg-[#029e4d] text-white font-bold px-[14px] py-[6px] rounded-[4px] border-b-[3px] border-[#028a42] active:translate-y-[1px] cursor-pointer">
          Sign In
        </button>
      </nav>

      <div
        className="h-[90vh] w-full bg-cover bg-center bg-no-repeat flex flex-row"
        style={{ backgroundImage: "url('/background.jpg')" }}
      > 
        <div className="flex flex-col w-1/2 h-full items-center justify-center pl-[120px]">
          <h2 className={`${luckiest.className} text-[120px] font-bold text-white [-webkit-text-stroke:5px_#f03f00] z-999`}>Pyxion</h2>
          <div className="bg-[rgb(0,0,0,0.75)] p-[25px] absolute bottom-55 font-extralight">
            <h1 className="text-3xl text-white t">Powering Fun</h1>
          </div>
        </div>

        <div className="flex flex-col w-1/2 h-full items-center justify-center">
          <div className="w-3/4 h-3/4 bg-[#ffffff] flex flex-col justify-center items-center">
            <h1 className="text-3xl text-[rgb(75,75,75)] pb-[15px] font-extralight mt-[20px]">Sign up and start having fun! </h1>
            <input 
              type="text" 
              className="flex w-[90%] h-12 outline-none border border-solid border-[rgb(200,200,200)] text-[20px] rounded-[5px] pl-[5px] mb-[15px] transition-colors duration-200 focus:border-[#00a0ee]"
              placeholder="Username (3-20 characters)" 
            />
            <input 
              type="password" 
              className="flex w-[90%] h-12 outline-none border border-solid border-[rgb(200,200,200)] text-[20px] rounded-[5px] pl-[5px] mb-[15px] transition-colors duration-200 focus:border-[#00a0ee]"
              placeholder="Password (8-12 characters)" 
            />
            <input 
              type="password" 
              className="flex w-[90%] h-12 outline-none border border-solid border-[rgb(200,200,200)] text-[20px] rounded-[5px] pl-[5px] mb-[15px] transition-colors duration-200 focus:border-[#00a0ee]"
              placeholder="Confirm Password" 
            />
            <div className="flex flex-row font-light text-[rgb(100,100,100)] items-center border border-solid border-[rgb(200,200,200)] w-[90%] h-12 rounded-[5px] mb-[15px]">
              <h1 className="flex items-center border-r border-solid border-[rgb(200,200,200)] text-[20px] h-full pl-[5px] pr-[20px]">
                Birthday
              </h1>
              <Dropdown
                options={MONTHS}
                value={month}
                placeholder="Month"
                onChange={setMonth}
                className="flex-[2] h-full border-r border-solid border-[rgb(200,200,200)] hover:bg-[#00a0ee] hover:text-[#fff]"
              />
              <Dropdown
                options={DAYS}
                value={day}
                placeholder="Day"
                onChange={setDay}
                className="flex-1 h-full border-r border-solid border-[rgb(200,200,200)] hover:bg-[#00a0ee] hover:text-[#fff]"
              />
              <Dropdown
                options={YEARS}
                value={year}
                placeholder="Year"
                onChange={setYear}
                className="flex-1 h-full hover:bg-[#00a0ee] hover:text-[#fff]"
              />
            </div>
            <div className="flex flex-row font-light text-[rgb(100,100,100)] items-center border border-solid border-[rgb(200,200,200)] w-[90%] h-12 rounded-[5px] mb-auto">
              <h1 className="flex items-center border-r border-solid border-[rgb(200,200,200)] text-[20px] h-full pl-[5px] pr-[20px]">
                Gender
              </h1>

              <button
                type="button"
                onClick={() => setGender("male")}
                className="flex-1 h-full flex items-center justify-center"
              >
                <img
                  src={gender === "male" ? "/icons/female-gray.png" : "/icons/male-gray.png"}
                  alt="Male"
                  className="w-[32px] h-[32px]"
                />
              </button>

              <button
                type="button"
                onClick={() => setGender("female")}
                className="flex-1 h-full flex items-center justify-center border-l border-solid border-[rgb(200,200,200)]"
              >
                <img
                  src={gender === "female" ? "/icons/female-color.png" : "/icons/male-color.png"}
                  alt="Female"
                  className="w-[32px] h-[32px]"
                />
              </button>
            </div>
            <button className="w-[90%] h-12 rounded-[5px] mb-auto bg-[#00d040] text-white cursor-pointer hover:bg-[#00f060] transition-colors duration-200">Sign Up</button>
          </div>
        </div>
      </div>
      <div className="h-[80vh] border-b-[5px] border-t-[5px] border-white"/>
      <div className="h-[80vh]"/>
    </div>
  );
}