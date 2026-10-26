"use client";

import { useState, type ChangeEvent } from "react";
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
  // Username check
  const [username, setUsername] = useState<string>("");
  const isTooShort = username.length > 0 && username.length < 3;
  const isTooLong = username.length > 20;

  // Password checks
  const [password, setPassword] = useState<string>("");
  const [confirmPassword, setConfirmPassword] = useState<string>("");
  const passShort = password.length > 0 && password.length < 8;
  const passLong = password.length > 12;
  const passMatch = password === confirmPassword;

  // Dropdown and gender
  const [month, setMonth] = useState("");
  const [day, setDay] = useState("");
  const [year, setYear] = useState("");
  const [gender, setGender] = useState<"male" | "female" | "">("");

  return (
    <div className="flex flex-col min-h-screen font-sans bg-[#000]">

      <nav className="h-[10vh] w-full bg-[#393b3d] text-white px-[10px] flex flex-row items-center gap-[14px] font-sans">
        <h2 className={`${luckiest.className} mr-auto text-[40px] font-bold text-white [-webkit-text-stroke:2px_#f03f00]`}>
          Pyxion
        </h2>
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
        <div className="relative flex flex-col w-1/2 h-full justify-center pl-[120px]">
          <h2 className={`${luckiest.className} text-[120px] font-bold text-white [-webkit-text-stroke:5px_#f03f00] z-50`}>
            Pyxion
          </h2>
          <div className="absolute bottom-[220px] left-[120px] bg-black/75 p-[25px] font-extralight">
            <h1 className="text-3xl text-white">Powering Fun</h1>
          </div>
        </div>

        <div className="flex flex-col w-1/2 h-full items-center justify-center">
          <div className="w-3/4 h-[90%] bg-white flex flex-col justify-center items-center">
            <h1 className="text-3xl text-[rgb(75,75,75)] pb-[15px] font-extralight mt-[20px]">
              Sign up and start having fun!
            </h1>

            {/* Username */}
            <input
              type="text"
              className="flex w-[90%] h-12 outline-none border border-solid border-[rgb(200,200,200)] text-[20px] rounded-[5px] mb-[15px] pl-[5px] transition-colors duration-200 focus:border-[#00a0ee]"
              placeholder="Username (3-20 characters)"
              value={username}
              onChange={(e: ChangeEvent<HTMLInputElement>) => setUsername(e.target.value)}
            />
            <p className="mb-[15px] text-red-500">
              {isTooShort ? "Username too short!" : ""}
              {isTooLong ? "Username too long!" : ""}
            </p>

            {/* Password */}
            <input
              type="password"
              className="flex w-[90%] h-12 outline-none border border-solid border-[rgb(200,200,200)] text-[20px] rounded-[5px] pl-[5px] mb-[15px] transition-colors duration-200 focus:border-[#00a0ee]"
              placeholder="Password (8-12 characters)"
              value={password}
              onChange={(e: ChangeEvent<HTMLInputElement>) => setPassword(e.target.value)}
            />
            <p className={`text-red-500 ${passLong ? "mb-[0px] h-[0px]" : "mb-[15px]"}`}>
              {passShort ? "Password too short!" : ""}
              {passLong ? "Password too long!" : ""}
            </p>

            {/* Confirm password */}
            <input
              type="password"
              className="flex w-[90%] h-12 outline-none border border-solid border-[rgb(200,200,200)] text-[20px] rounded-[5px] pl-[5px] mb-[15px] transition-colors duration-200 focus:border-[#00a0ee]"
              placeholder="Confirm Password"
              value={confirmPassword}
              onChange={(e: ChangeEvent<HTMLInputElement>) => setConfirmPassword(e.target.value)}
            />
            <p className="text-red-500 mb-[15px]">
              {passMatch ? "" : "Password does not match"}
            </p>

            {/* Birthday */}
            <div className="flex flex-row font-light text-[rgb(100,100,100)] items-center border border-solid border-[rgb(200,200,200)] w-[90%] h-12 rounded-[5px] mb-[15px]">
              <h1 className="flex items-center border-r border-solid border-[rgb(200,200,200)] text-[20px] h-full pl-[5px] pr-[20px]">
                Birthday
              </h1>
              <Dropdown
                options={MONTHS}
                value={month}
                placeholder="Month"
                onChange={setMonth}
                className="flex-[2] h-full border-r border-solid border-[rgb(200,200,200)] hover:bg-[#00a0ee] hover:text-white"
              />
              <Dropdown
                options={DAYS}
                value={day}
                placeholder="Day"
                onChange={setDay}
                className="flex-1 h-full border-r border-solid border-[rgb(200,200,200)] hover:bg-[#00a0ee] hover:text-white"
              />
              <Dropdown
                options={YEARS}
                value={year}
                placeholder="Year"
                onChange={setYear}
                className="flex-1 h-full hover:bg-[#00a0ee] hover:text-white"
              />
            </div>

            {/* Gender */}
            <div className="flex flex-row font-light text-[rgb(100,100,100)] items-center border border-solid border-[rgb(200,200,200)] w-[90%] h-12 rounded-[5px] mb-[15px]">
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

            <button className="w-[90%] h-12 rounded-[5px] mb-[20px] bg-[#00d040] text-white cursor-pointer hover:bg-[#00f060] transition-colors duration-200">
              Sign Up
            </button>
          </div>
        </div>
      </div>

      <div className="h-[80vh] border-b-[5px] border-t-[5px] border-white" />
      <div className="h-[80vh]" />
    </div>
  );
}