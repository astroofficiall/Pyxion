"use client";

import { useEffect, useRef, useState } from "react";

type DropdownProps = {
  options: string[];
  value: string;
  placeholder: string;
  onChange: (value: string) => void;
  className?: string;
};

export default function Dropdown({
  options,
  value,
  placeholder,
  onChange,
  className = "",
}: DropdownProps) {
  const [open, setOpen] = useState(false);
  const ref = useRef<HTMLDivElement>(null);

  // Close when clicking anywhere outside this dropdown
  useEffect(() => {
    function handleClickOutside(e: MouseEvent) {
      if (ref.current && !ref.current.contains(e.target as Node)) {
        setOpen(false);
      }
    }
    document.addEventListener("mousedown", handleClickOutside);
    return () => document.removeEventListener("mousedown", handleClickOutside);
  }, []);

  return (
    <div ref={ref} className={`relative ${className}`}>
      <button
        type="button"
        onClick={() => setOpen((o) => !o)}
        className="flex w-full h-full items-center justify-between px-[10px] cursor-pointer text-[rgb(75,75,75)] hover:bg-[#00a0ee] hover:text-white"
      >
        <span className={value ? "" : "opacity-60"}>
          {value || placeholder}
        </span>
        <svg
          width="16"
          height="16"
          viewBox="0 0 24 24"
          fill="none"
          stroke="currentColor"
          strokeWidth="2"
          className={`transition-transform duration-200 ${open ? "rotate-180" : ""}`}
        >
          <path d="M6 9l6 6 6-6" />
        </svg>
      </button>

      {open && (
        <ul className="absolute left-0 top-full mt-[2px] w-full max-h-[280px] overflow-y-auto bg-white shadow-[0_2px_8px_rgba(0,0,0,0.3)] rounded-[4px] py-[4px] z-50">
          {options.map((opt) => (
            <li key={opt}>
              <button
                type="button"
                onClick={() => {
                  onChange(opt);
                  setOpen(false);
                }}
                className={`w-full text-left px-[12px] py-[6px] text-[18px] text-[rgb(75,75,75)] hover:bg-[#e6e6e6] cursor-pointer ${
                  opt === value ? "font-bold" : ""
                }`}
              >
                {opt}
              </button>
            </li>
          ))}
        </ul>
      )}
    </div>
  );
}